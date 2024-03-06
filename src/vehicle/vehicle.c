#include "../common.h"
#include "../fs/bsp.h"
#include "../game/game.h"
#include "../physics/physics.h"

#include "vehicle.h"

const char* vehicle_ids[vehicle_max_id] =
{
	"sdf_kart", "mdf_kart", "ldf_kart", "sa_kart", "ma_kart", "la_kart", 
	"sb_kart",  "mb_kart",  "lb_kart",  "sc_kart", "mc_kart", "lc_kart", 
	"sd_kart",  "md_kart",  "ld_kart",  "se_kart", "me_kart", "le_kart",  
	
	"sdf_bike", "mdf_bike", "ldf_bike", "sa_bike", "ma_bike", "la_bike",
	"sb_bike",  "mb_bike",  "lb_bike",  "sc_bike", "mc_bike", "lc_bike",
	"sd_bike",  "md_bike",  "ld_bike",  "se_bike", "me_bike", "le_bike",
};

void vehicle_collision_init(vehicle_collision_t* collision)
{
	memset(collision, 0, sizeof(*collision));
	collision->speed_factor = 1.0;
}

void vehicle_collision_add(vehicle_collision_t* collision, vehicle_t* vehicle, collision_t* kcl_collision)
{
	collision->valid = true;
	collision->count++;

	vec3_add(&collision->floor_normal, &kcl_collision->floor_normal, &collision->floor_normal);

	/* TODO label these masks */
	hit_t* hit = collision_find_furthest(kcl_collision, 0x20E80FFF);
	if (hit)
	{
		if (hit->surface & 0x2000)
			collision->has_trickable = true;

		uint16_t kind = hit->surface & 0x1F;
		collision->speed_factor  = fminf(collision->speed_factor, vehicle->stats.speed_multipliers[kind]);
		collision->rot_factor	+= vehicle->stats.rotation_multipliers[kind];

		if (!collision->has_trickable)
		{
			if ((hit->surface & 0x2000)
				|| collision_find_furthest(kcl_collision, 0x100))
			{
				collision->has_trickable = true;
			}
		}
	}
}

void vehicle_collision_set_normal(vehicle_collision_t* collision, vec3_t* normal)
{
	collision->valid = true;
	collision->floor_normal = *normal;
	collision->rot_factor = 1.0f;
}

void vehicle_collision_finalize(vehicle_collision_t* collision)
{
	if (collision->valid)
		vec3_norm(&collision->floor_normal);

	if (collision->count > 0)
		collision->rot_factor /= (float)collision->count;
}


void vehicle_wheel_init(vehicle_wheel_t* wheel, bsp_wheel_t* bsp, bikepart_handle_t* handle, vec3_t* pos, int index)
{
	wheel->handle     = handle;
	wheel->bsp		  = *bsp;
	wheel->axis       = vec3_down;
	wheel->axis_s     = wheel->bsp.suspension_slack;

	if (index % 2 == 1)
		wheel->bsp.suspension_top.x *= -1.0f;

	vec3_add(&wheel->bsp.suspension_top, pos, &wheel->topmost_pos);
	vec3_mul(&wheel->axis, wheel->axis_s, &wheel->pos);
	vec3_add(&wheel->pos, &wheel->topmost_pos, &wheel->pos);
	wheel->last_pos = wheel->pos;
	vec3_sub(&wheel->pos, &wheel->topmost_pos, &wheel->last_pos_rel);

	hitbox_init(&wheel->hitbox, &wheel->pos, pos, 10.0f, 0x20E80FFF);
	wheel->hitbox_pos_rel = vec3_zero;

	vehicle_collision_init(&wheel->collision);
}

bool vehicle_wheel_update(vehicle_wheel_t* wheel, vehicle_t* vehicle, kcl_t* kcl, vec3_t* out)
{
	bsp_wheel_t* bsp_wheel = &wheel->bsp;
	physics_t* physics = &vehicle->physics;
	mat34_t mat34;
	mat33_t mat33;

	vehicle_wheel_mat(wheel, physics, &mat34);
	mat33_init_mat34(&mat33, &mat34);

	wheel->axis_s = fminf(wheel->axis_s + 5.0f, bsp_wheel->suspension_slack);
	mat34_mulv(&mat34, &bsp_wheel->suspension_top, &wheel->topmost_pos);
	mat33_mulv(&mat33, &vec3_down, &wheel->axis);
	wheel->last_pos = wheel->pos;
	vec3_mul(&wheel->axis, wheel->axis_s, &wheel->pos);
	vec3_add(&wheel->topmost_pos, &wheel->pos, &wheel->pos);

	vec3_t hitbox_pos;
	float radius_diff = bsp_wheel->radius - bsp_wheel->sphere_radius;
	vec3_mul(&wheel->axis, radius_diff, &hitbox_pos);
	vec3_add(&wheel->pos, &hitbox_pos, &hitbox_pos);

	if (vehicle_is_bike(vehicle))
	{
		vec3_t right;
		mat33_init_mat34(&mat33, &physics->mat);
		mat33_mulv(&mat33, &vec3_right, &right);
		vec3_mul(&right, vehicle->lean.rot * bsp_wheel->sphere_radius * 0.3f, &right);
		vec3_add(&hitbox_pos, &right, &hitbox_pos);
	}

	hitbox_update_pos(&wheel->hitbox, &hitbox_pos);
	vec3_sub(&hitbox_pos, &physics->pos, &wheel->hitbox_pos_rel);

	collision_t collision;
	vec3_t movement;
	kcl_collision_hitbox(kcl, &wheel->hitbox, &collision);
	collision_movement(&collision, &movement);

	vec3_add(&wheel->pos, &movement, &wheel->pos);
	wheel->hitbox.radius = bsp_wheel->sphere_radius;
	
	vehicle_collision_init(&wheel->collision);
	if (collision.surface_kinds & 0x20E80FFF)
	{
		vehicle_collision_add(&wheel->collision, vehicle, &collision);
		surface_props_add(&vehicle->surface_props, &collision, true);
	}
	
	vec3_t delta;
	vec3_sub(&wheel->pos, &wheel->topmost_pos, &delta);
	wheel->axis_s = vec3_dot(&wheel->axis, &delta);

	if (wheel->axis_s < 0.0f)
	{
		vec3_mul(&wheel->axis, wheel->axis_s, out);
		return true;
	}

	return false;
}

void vehicle_wheel_update_suspension(vehicle_wheel_t* wheel, vehicle_t* vehicle, vec3_t* movement)
{
	bsp_wheel_t* bsp_wheel = &wheel->bsp;
	physics_t* physics = &vehicle->physics;

	vec3_t delta;
	vec3_t topmost_pos = wheel->topmost_pos;
	vec3_add(&wheel->topmost_pos, movement, &wheel->topmost_pos);
	vec3_sub(&wheel->pos, &wheel->topmost_pos, &delta);
	wheel->axis_s = clampf(vec3_dot(&wheel->axis, &delta), 0.0f, bsp_wheel->suspension_slack);
	vec3_mul(&wheel->axis, wheel->axis_s, &delta);
	vec3_add(&wheel->topmost_pos, &delta, &wheel->pos);

	if (wheel->collision.valid)
	{
		vec3_t pos_rel, acceleration, topmost_pos_rel, cross, temp;

		vec3_sub(&wheel->pos, &topmost_pos, &pos_rel);
		vec3_sub(&wheel->last_pos_rel, &pos_rel, &delta);

		float dist = bsp_wheel->suspension_slack - fmaxf(vec3_dot(&wheel->axis, &pos_rel), 0.0f);
		float dist_acceleration = -bsp_wheel->suspension_distance * dist;
		float speed = vec3_dot(&wheel->axis, &delta);
		float speed_acceleration = -bsp_wheel->suspension_speed * speed;
		vec3_mul(&wheel->axis, dist_acceleration + speed_acceleration, &acceleration);

		if (!vehicle->jump_pad.applied_dir && physics->vel0.y <= 5.0f)
		{
			vec3_t acceleration_dir = { acceleration.x, 0.0f, acceleration.z };
			vec3_proj_unit(&acceleration_dir, &wheel->collision.floor_normal, &temp);
			float normal_acceleration = acceleration.y + temp.y;
			float max_normal_acceleration = vehicle->stats.max_normal_acceleration;
			physics->normal_acceleration += fminf(normal_acceleration, max_normal_acceleration);
		}

		temp = acceleration;
		vec3_sub(&topmost_pos, &physics->pos, &delta);
		quat_inv_rotate(&physics->full_rot, &delta, &topmost_pos_rel);
		quat_inv_rotate(&physics->full_rot, &temp, &acceleration);
		vec3_cross(&topmost_pos_rel, &acceleration, &cross);

		if (vehicle_is_bike(vehicle) && vehicle->wheelie.rot > 0.0)		
			cross.x = 0.0f;

		cross.y = 0.0f;
		vec3_add(&physics->normal_rot_vec, &cross, &physics->normal_rot_vec);
	}

	vec3_sub(&wheel->pos, &wheel->topmost_pos, &wheel->last_pos_rel);

	if (wheel->collision.valid)
	{
		vec3_t* floor_nor = &wheel->collision.floor_normal;

		vec3_t vel, temp;
		vec3_sub(&wheel->pos, &wheel->last_pos, &vel);
		vec3_sub(&vel, &physics->vel1, &vel);

		vec3_mul(&vec3_down, 10.0f * 1.3f, &temp);
		vec3_add(&vel, &temp, &temp);

		float dot = vec3_dot(&temp, floor_nor);
		if (dot < 0.0f)
		{
			vec3_t cross, temp2;
			vec3_mul(&vel, -1.0f, &temp);
			vec3_cross(floor_nor, &temp, &temp2);
			vec3_cross(&temp2, floor_nor, &cross);

			if (vec3_magsqr(&cross) > FLT_EPSILON)
			{
				mat34_t mat34, tensor, transpose;
				mat33_t mat33;
				vec3_t front, other_cross, proj, rej, sum;

				mat34_init_quat_pos(&mat34, &physics->main_rot, &vec3_zero);
				mat34_transpose(&mat34, &transpose);
				mat34_mulm(&mat34, &physics->inv_inertia_tensor, &tensor);
				mat34_mulm(&tensor, &transpose, &mat34);
				mat33_init_mat34(&mat33, &mat34);	
			
				vec3_cross(&wheel->hitbox_pos_rel, floor_nor, &temp);
				mat33_mulv(&mat33, &temp, &temp2);
				vec3_cross(&temp2, &wheel->hitbox_pos_rel, &other_cross);

				vec3_norm(&cross);
				/*
				* Warning: do not fold the expressions or risk consequences!
				* MSVC will compute these in x87 at 80-bit precision which is incorrect
				* I.e. folding the lim variable like this will cause a desync
				* float scale = (val * fminf(vec3_dot(&vel, &cross), 0.0f)) / dot;
				*/
				float val = -dot / (1.0f + vec3_dot(floor_nor, &other_cross));
				float lim = fminf(vec3_dot(&vel, &cross), 0.0f);
				float scale = (val * lim) / dot;
				vec3_mul(&cross, scale, &cross);

				quat_rotate(&physics->full_rot, &vec3_front, &front);

				vec3_proj_unit(&cross, &front, &proj);
				float proj_norm = vec3_magu(&proj);
				proj_norm = copysignf(1.0f, proj_norm) * fminf(fabsf(proj_norm), 0.1f * fabsf(val));
				vec3_norm(&proj);
				vec3_mul(&proj, proj_norm, &proj);

				vec3_rej_unit(&cross, &front, &rej);
				float rej_norm = vec3_magu(&rej);
				rej_norm = copysignf(1.0f, rej_norm) * fminf(fabsf(rej_norm), 0.8f * fabsf(val));
				vec3_norm(&rej);
				vec3_mul(&rej, rej_norm, &rej);

				vec3_add(&proj, &rej, &sum);
				vec3_rej_unit(&sum, &physics->dir, &rej);
				vec3_add(&physics->vel0, &rej, &physics->vel0);

				if (!vehicle_is_bike(vehicle) || vehicle->wheelie.rot <= 0.0f )
				{
					vec3_cross(&wheel->hitbox_pos_rel, &sum, &temp);
					mat33_mulv(&mat33, &temp, &temp2);
					quat_inv_rotate(&physics->main_rot, &temp2, &cross);
					cross.y = 0.0f;
					vec3_add(&physics->rot_vec0, &cross, &physics->rot_vec0);
				}
			}
		}
	}
}

void vehicle_wheel_mat(vehicle_wheel_t* wheel, physics_t* physics, mat34_t* mat)
{
	mat34_init_quat_pos(mat, &physics->full_rot, &physics->pos);
	if (wheel->handle)
	{
		mat34_t old_mat = *mat, handle_mat;
		mat34_init_angles_pos(&handle_mat, &wheel->handle->angles, &wheel->handle->pos);
		mat34_mulm(&old_mat, &handle_mat, mat);
	}
}

void vehicle_wheel_dump_state(vehicle_wheel_t* wheel)
{
	printf("w: topmost_pos %.10f %.10f %.10f\n", XYZ(wheel->topmost_pos));
	printf("w: pos %.10f %.10f %.10f\n", XYZ(wheel->pos));
	printf("w: axis %.10f %.10f %.10f\n", XYZ(wheel->axis));
	printf("w: axis_s %.10f\n", wheel->axis_s);
	if (wheel->collision.valid)
		printf("w: rotfactor %.10f\n", wheel->collision.rot_factor);
	else
		printf("w: rotfactor NULL\n");
}

void vehicle_body_init(vehicle_body_t* body, bsp_t* bsp, mat34_t* mat)
{
	body->hitboxes = malloc(bsp->hitbox_count * sizeof(*body->hitboxes));
	for (int i = 0; i < bsp->hitbox_count; i++)
	{
		hitbox_t*     hitbox     = &body->hitboxes[i];
		bsp_hitbox_t* bsp_hitbox = &bsp->hitboxes[i];

		hitbox->pos    = vec3_zero;
		mat34_mulv(mat, &bsp_hitbox->sphere_center, &hitbox->last_pos);
		hitbox->last_pos_valid = true;
		hitbox->radius = bsp_hitbox->sphere_radius;
		hitbox->flags  = 0x20E80FFF;
	}

	body->bsp_hitboxes		  = bsp->hitboxes;
	body->hitbox_count        = bsp->hitbox_count;
	vehicle_collision_init(&body->collision);
	body->has_floor_collision = false;
}

void vehicle_body_free(vehicle_body_t* body)
{
	free(body->hitboxes);
	body->bsp_hitboxes = NULL;
	body->hitbox_count = 0;
	body->hitboxes = NULL;
}

void vehicle_body_update(vehicle_body_t* body, vehicle_t* vehicle, kcl_t* kcl)
{
	physics_t* physics = &vehicle->physics;

	vec3_t min = vec3_zero;
	vec3_t max = vec3_zero;
	vec3_t pos_rel = vec3_zero;

	vehicle_collision_t* collision = &body->collision;
	vehicle_collision_init(collision);

	for (int i = 0; i < body->hitbox_count; i++)
	{
		bsp_hitbox_t* bsp_hitbox = &body->bsp_hitboxes[i];
		hitbox_t* hitbox         = &body->hitboxes[i];

		if (bsp_hitbox->wall_only)
			continue;

		vec3_t hitbox_pos_rel, pos;
		quat_rotate(&physics->full_rot, &bsp_hitbox->sphere_center, &hitbox_pos_rel);

		vec3_add(&hitbox_pos_rel, &physics->pos, &pos);
		hitbox_update_pos(hitbox, &pos);

		collision_t hitbox_collision;
		kcl_collision_hitbox(kcl, hitbox, &hitbox_collision);

		if (hitbox_collision.surface_kinds & 0x20E80FFF)
		{
			vec3_t movement, dir, sphere, temp;
			collision_movement(&hitbox_collision, &movement);
			vec3_min(&min, &movement, &min);
			vec3_max(&max, &movement, &max);

			dir = movement;
			vec3_norm(&dir);

			vec3_mul(&dir, bsp_hitbox->sphere_radius, &sphere);
			vec3_add(&pos_rel, &hitbox_pos_rel, &temp);
			vec3_sub(&temp, &sphere, &pos_rel);

			vehicle_collision_add(collision, vehicle, &hitbox_collision);
			surface_props_add(&vehicle->surface_props, &hitbox_collision, false);
		}
	}

	uint8_t count = collision->count;
	body->has_floor_collision = count > 0;

	if (count > 0)
	{
		vec3_t movement;
		vec3_add(&min, &max, &movement);
		vec3_add(&physics->pos, &movement, &physics->pos);

		vehicle_collision_finalize(collision);
		vec3_mul(&pos_rel, 1.0f / (float)count, &pos_rel);

		vec3_t pos_rel_r, rot_vec0, cross, vel;
		vec3_mul(&physics->rot_vec0, physics->rot_factor, &rot_vec0);
		quat_inv_rotate(&physics->main_rot, &pos_rel, &pos_rel_r);
		vec3_cross(&rot_vec0, &pos_rel_r, &cross);
		quat_rotate(&physics->main_rot, &cross, &vel);
		vec3_add(&vel, &physics->vel0, &vel);

		if (physics->vel1.y > 0.0f)
			vel.y += physics->vel1.y;

		if (collision->valid)
			physics_update_rigid_body(physics, vehicle, &pos_rel, &vel, &collision->floor_normal);
	}
}

void vehicle_init(vehicle_t* vehicle, bsp_t* bsp, game_t* game, uint8_t vehicle_id, uint8_t character_id)
{ 
	vehicle->id = vehicle_id;

	param_merge(
		&game->kartparam.sections[vehicle_id], 
		&game->driverparam.sections[character_id], 
		&vehicle->stats);

	if (vehicle_is_bike(vehicle))
		vehicle->bikeparts = &game->bikeparts.sections[vehicle_id - vehicle_bike_id];
	else
		vehicle->bikeparts = NULL;

	vehicle->handle	   = NULL;

	vehicle->player	   = NULL;
	vehicle->bsp	   = bsp;

	physics_init       (&vehicle->physics);
	floor_init         (&vehicle->floor);
	surface_props_init (&vehicle->surface_props);

	turn_init          (&vehicle->turn);
	drift_init         (&vehicle->drift);
	wheelie_init       (&vehicle->wheelie);
	lean_init		   (&vehicle->lean);
	dive_init          (&vehicle->dive);

	vehicle_body_init  (&vehicle->body, vehicle->bsp, &vehicle->physics.mat);

	vehicle->wheel_count = 0;

	start_boost_init   (&vehicle->start_boost);
	standstill_boost_init(&vehicle->standstill_boost);
	standstill_miniturbo_init(&vehicle->standstill_miniturbo);
	ramp_boost_init    (&vehicle->ramp_boost);
	boost_init         (&vehicle->boost);

	trick_init         (&vehicle->trick);
	jump_pad_init      (&vehicle->jump_pad);

	bool inside_drift = vehicle_is_inside_drift(vehicle);
	bool is_bike = vehicle_is_bike(vehicle);
	drift_set_drift(&vehicle->drift, inside_drift, !is_bike);
	lean_set_drift(&vehicle->lean, inside_drift);
}

void vehicle_free(vehicle_t* vehicle)
{
	vehicle_body_free(&vehicle->body);
	free(vehicle->bsp);
	vehicle->bikeparts		 = NULL;
	vehicle->bsp			 = NULL;
	vehicle->player			 = NULL;
	vehicle->wheel_count     = 0;
}

vehicle_t* vehicle_load(player_t* player, game_t* game, uint8_t vehicle_id, uint8_t character_id)
{
	const char* vehicle_name = vehicle_name_by_id(vehicle_id);
	if (!vehicle_name[0])
	{
		printf("Failed to load vehicle, bad ID %d\n", vehicle_id);
		return NULL;
	}

	/* TODO load from bsp folder */
	/* TODO should be shared between vehicles, fine for now with 1 player */

	char bsp_name[_MAX_PATH];
	sprintf(bsp_name, "%s.bsp", vehicle_name);

	bin_t* bsp_data = arc_find_data(&game->common, bsp_name);
	if (!bsp_data)
	{
		printf("Failed to load vehicle, missing %s\n", bsp_name);
		return NULL;
	}

	bsp_t* bsp = malloc(sizeof(bsp_t));
	if (!bsp_parser.parse(bsp, bsp_data))
	{
		bsp_parser.free(bsp);
		return NULL;
	}

	vehicle_t* vehicle = malloc(sizeof(vehicle_t));
	vehicle_init(vehicle, bsp, game, vehicle_id, character_id);

	player->vehicle = vehicle;
	vehicle->player = player;
	return vehicle;
}

typedef struct
{
	int wheel_count;
	bool has_handle;
} vehicle_tire_t;

const vehicle_tire_t vehicle_tire_map[] =
{
	{ 4, false },
	{ 2, true  },
	{ 2, false },
	{ 3, false },
};
const vehicle_tire_t vehicle_tire_invalid = { 0, false };

void vehicle_place(vehicle_t* vehicle, course_t* course)
{
	physics_place(&vehicle->physics, vehicle->bsp, &course->kmp, &course->kcl);

	/* TODO is this the best place to set it up? */
	const vehicle_tire_t* tire_map;
	int32_t tire_index = vehicle->stats.num_tires;
	if (tire_index < ARRAY_LEN(vehicle_tire_map))
		tire_map = &vehicle_tire_map[tire_index];
	else
		tire_map = &vehicle_tire_invalid;

	if (vehicle->bikeparts && tire_map->has_handle)
		vehicle->handle = &vehicle->bikeparts->handle;

	for (int i = 0; i < 4; i++)
	{
		if (tire_map->wheel_count == 2 && i % 2 != 0)
			continue;
		if (tire_map->wheel_count == 3 && i == 0)
			continue;

		vehicle_wheel_t* wheel = &vehicle->wheels[vehicle->wheel_count++];
		vehicle_wheel_init(wheel,
			&vehicle->bsp->wheels[i / 2],
			i == 0 ? vehicle->handle : NULL,
			&vehicle->physics.pos,
			i);
	}
}

const char* vehicle_name_by_id(uint8_t id)
{
	if (id > ARRAY_LEN(vehicle_ids))
		return "";
	return vehicle_ids[id];
}
