#include "../common.h"
#include "player.h"

#include "../fs/arc.h"
#include "../fs/rkg.h"
#include "../fs/param.h"
#include "../course/course.h"
#include "../game/game.h"

#include "../vehicle/vehicle.h"
#include "../physics/physics.h"

#include "../graphics/graphics.h"

void player_init(player_t* player)
{
	player->vehicle = NULL;
	memset(&player->input, 0, sizeof(player->input));
	memset(&player->input_last, 0, sizeof(player->input_last));
	memset(&player->last_key_state, 0, sizeof(player->last_key_state));
	player->freecam = false;
}

void player_free(player_t* player)
{
	player->vehicle = NULL;
	free(player);
}

int player_load_ghost(player_t* player, game_t* game, rkg_t* rkg)
{
	rkg_header_t* header = &rkg->header;

	vehicle_t* vehicle = vehicle_load(player, game, header->vehicle_id, header->character_id);
	if (!vehicle)
		return 0;

	vehicle_place(vehicle, &game->course);
	game_add_player(game, player);
	return 1;
}	

void player_update(player_t* player, game_t* game)
{
	vehicle_t* vehicle		       = player->vehicle;
	input_t*   input		       = &player->input;
	physics_t* physics		       = &vehicle->physics;
	floor_t*   floor		       = &vehicle->floor;
	trick_t*   trick			   = &vehicle->trick;
	surface_props_t* surface_props = &vehicle->surface_props;
	kcl_t*	   kcl				   = &game->course.kcl;
	bool	   is_bike			   = vehicle_is_bike(vehicle);

	int stage = game_get_stage(game); /* TODO: shouldnt this be stored off in game? */

	physics->rot_vec2 = vec3_zero;

	floor_update(floor, vehicle);

	if (floor->airtime == 0)
		trick_try_end(trick, vehicle);

	physics->gravity = -1.3f;

	standstill_miniturbo_try_start(&vehicle->standstill_miniturbo, vehicle, input);

	if (stage == stage_countdown)
		start_boost_update(&vehicle->start_boost, input->accelerate);
	else if (game->frame_idx == stage_frame_race + 1)
		boost_activate(&vehicle->boost, boost_weak, start_boost_frames(&vehicle->start_boost));

	physics_update_ups(physics, vehicle);

	if (floor_is_landing(floor))
		vehicle->jump_pad.variant = variant_invalid;

	if (surface_props->has_boost_panel)
	{
		boost_activate(&vehicle->boost, boost_strong, 60);
		floor_activate_invincibility(floor, 60);
	}

	if (surface_props->has_boost_ramp)
		vehicle->ramp_boost.duration = 60;

	jump_pad_update(&vehicle->jump_pad, physics, surface_props->jump_pad);

	trick_update_rot(trick, vehicle);

	trick_update_next(trick, vehicle, input->trick);

	physics_update_dirs(physics, vehicle);

	trick_try_start(trick, vehicle);

	physics_update_landing_angle(physics);

	floor_update_sticky(floor, vehicle, kcl);

	floor_update_factors(floor, vehicle);

	turn_update(&vehicle->turn, vehicle, input->stick_x);

	drift_update(&vehicle->drift, vehicle, input->stick_x, input->drift && stage == stage_race, player->input_last.drift);

	if (is_bike)
		wheelie_update(&vehicle->wheelie, vehicle, input->trick);

	standstill_miniturbo_update(&vehicle->standstill_miniturbo, vehicle);

	boost_update(&vehicle->boost);

	ramp_boost_update(&vehicle->ramp_boost);

	if (floor->invincibility > 0)
		floor->invincibility--;

	physics_update_accel(physics, vehicle, input, stage);

	standstill_boost_update(&vehicle->standstill_boost, vehicle, stage);

	if (is_bike)
	{
		physics->rot_vec2.x += vehicle->standstill_boost.rotation;

		lean_update(&vehicle->lean, vehicle, input->stick_x, stage);
	}
	else
	{
		physics->rot_vec0.x += vehicle->standstill_boost.rotation;

		float norm = 0.0f;
		if (floor->airtime == 0)
		{
			vec3_t front, front_perp, rej, perp;
			mat33_t m;

			mat33_init_mat34(&m, &physics->mat);
			mat33_mulv(&m, &vec3_front, &front);
			vec3_cross_plane(&front, &physics->up, &front_perp);
			vec3_norm(&front_perp);
			vec3_rej_unit(&physics->vel, &front_perp, &rej);
			vec3_cross_plane(&rej, &physics->up, &perp);

			float sq_norm = vec3_magsqr(&perp);
			if (sq_norm > FLT_EPSILON)
			{
				float det = perp.x * front_perp.z - perp.z * front_perp.x;
				norm = -fminf(sqrtf(sq_norm), 1.0f) * copysignf(1.0f, det);
			}
		}
		else if (vehicle->drift.state != drift_hop || vehicle->drift.hop.pos_y <= 0.0f)
		{
			physics->rot_vec0.z *= 0.98f;
		}

		physics->rot_vec0.z += vehicle->stats.tilt * norm * fabsf(vehicle->turn.raw);
	}

	turn_update_rot(&vehicle->turn, vehicle, input);

	float stick_y;
	if (stage != stage_race)
		stick_y = 0.0f;
	else
		stick_y = input->stick_y;

	dive_update(&vehicle->dive, vehicle, stick_y);

	physics_update(physics, vehicle, stage);

	surface_props_reset(surface_props);

	vehicle_body_update(&vehicle->body, vehicle, kcl);

	int count = 0;
	vec3_t min = vec3_zero;
	vec3_t max = vec3_zero;
	vec3_t pos_rel = vec3_zero;
	vec3_t vel = vec3_zero;
	vec3_t floor_nor = vec3_zero;
	vec3_t movement, temp;

	for (int i = 0; i < vehicle->wheel_count; i++)
	{
		vehicle_wheel_t* wheel = &vehicle->wheels[i];

		if (vehicle_wheel_update(wheel, vehicle, kcl, &movement))
		{
			vec3_min(&min, &movement, &min);
			vec3_max(&max, &movement, &max);

			if (wheel->collision.valid)
			{
				count++;
				vec3_add(&pos_rel, &wheel->hitbox_pos_rel, &pos_rel);
				vec3_mul(&vec3_down, 10.0f * 1.3f, &temp);
				vec3_add(&vel, &temp, &vel);
				vec3_add(&floor_nor, &wheel->collision.floor_normal, &floor_nor);
			}
		}
	}

	vec3_add(&min, &max, &movement);
	vec3_add(&physics->pos, &movement, &physics->pos);

	if (count > 0 && !vehicle->body.collision.valid)
	{
		float recip = 1.0f / (float)count;
		vec3_mul(&pos_rel, recip, &pos_rel);
		vec3_mul(&vel, recip, &vel);
		vec3_norm(&floor_nor);
		
		physics_update_rigid_body(physics, vehicle, &pos_rel, &vel, &floor_nor);
		vehicle_collision_set_normal(&vehicle->body.collision, &floor_nor);
	}

	for (int i = 0; i < vehicle->wheel_count; i++)
	{
		vehicle_wheel_t* wheel = &vehicle->wheels[i];
		vehicle_wheel_update_suspension(wheel, vehicle, &movement);
	}

	drift_hop_update_physics(&vehicle->drift.hop);

	mat34_init_quat_pos(&physics->mat, &physics->full_rot, &physics->pos);

	if (player->input.use_item && !player->input_last.use_item)
	{
		boost_activate(&vehicle->boost, boost_strong, 90);
		floor_activate_invincibility(floor, 90);
		vehicle->boost.mushroom_boost = 90;
	}
	
	if (!player->freecam && game->graphics)
	{
		camera_t* camera = &game->graphics->camera;

		vec3_t camera_target_pos;
		vec3_muladd(&physics->pos, 200.0f, &vec3_up, &camera_target_pos);
		vec3_muladd(&camera_target_pos, -400.0f, &physics->dir, &camera_target_pos);
		vec3_mul(&camera_target_pos, -1.0f, &camera_target_pos);

		vec3_t camera_dir = physics->dir;
		camera_dir.y = 0.0f;
		vec3_norm(&camera_dir);

		vec3_t camera_angles_pitch = { 0.0f, atan2f(camera_dir.z, camera_dir.x) - radiansf(90.0f), 0.0f };
		quat_t camera_quat_pitch;
		quat_init_angles(&camera_quat_pitch, &camera_angles_pitch);
		quat_invert(&camera_quat_pitch, &camera_quat_pitch);

		vec3_t camera_angles_yaw = { radiansf(15.0f), 0.0f, 0.0f };
		quat_t camera_quat_yaw;
		quat_init_angles(&camera_quat_yaw, &camera_angles_yaw);

		quat_t camera_target_orient;
		quat_mulq(&camera_quat_pitch, &camera_quat_yaw, &camera_target_orient);

		vec3_t camera_final_pos;
		quat_t camera_final_orient;
		vec3_lerp(&camera->pos, &camera_target_pos, camera->pos_lerp, &camera_final_pos);
		quat_slerp(&camera->quat, &camera_target_orient, camera->rot_lerp, &camera_final_orient);
		
		camera_set_transform(camera, &camera_final_pos, &camera_final_orient);
		camera->pos_lerp = 0.90f;
		camera->rot_lerp = 0.15f;
	}
}