#include "../common.h"
#include "../vehicle/vehicle.h"

#include "floor.h"

void floor_init(floor_t* floor)
{
	memset(floor, 0, sizeof(*floor));
	floor->speed_factor    = 1.0f;
	floor->rotation_factor = 1.0f;
}

void floor_update(floor_t* floor, vehicle_t* vehicle)
{
	bool floor_trickable = false;

	floor->valid = false;
	floor->normal = vec3_zero;

	for (int i = 0; i < vehicle->wheel_count; i++)
	{
		vehicle_collision_t* wheel_collision = &vehicle->wheels[i].collision;

		if (wheel_collision->valid)
		{
			vec3_add(&floor->normal, &wheel_collision->floor_normal, &floor->normal);
			floor->valid = true;
		}

		floor_trickable |= wheel_collision->has_trickable;
	}

	vehicle_collision_t* body_collision = &vehicle->body.collision;
	if (body_collision->valid)
	{
		vec3_add(&floor->normal, &body_collision->floor_normal, &floor->normal);
		floor->valid = true;
	}

	floor_trickable |= body_collision->has_trickable;

	floor->last_airtime = floor->airtime;

	if (floor->valid)
	{
		vec3_norm(&floor->normal);
		floor->airtime = 0;
	}
	else
	{
		floor->airtime++;
	}

	if (floor->trickable_timer > 0)
		floor->trickable_timer--;

	if (floor->airtime == 0)
	{
		if (floor_trickable)
		{
			floor->trickable_timer = 3;
			floor->has_trickable = true;
		}
		else
		{
			floor->has_trickable = floor->trickable_timer > 0;
		}
	}
}

void floor_update_factors(floor_t* floor, vehicle_t* vehicle)
{
	/* TODO: does -1.0f make more sense here? */
	if (floor->invincibility > 0)
	{
		floor->speed_factor    = vehicle->stats.speed_multipliers[0];
		floor->rotation_factor = vehicle->stats.rotation_multipliers[0];
	}
	else
	{
		float speed_factor_min    = FLT_MAX;
		float rotation_factor_sum = 0.0f;
		int   collision_count     = 0;

		for (int i = 0; i < vehicle->wheel_count; i++)
		{
			vehicle_collision_t* wheel_collision = &vehicle->wheels[i].collision;
			if (wheel_collision->valid)
			{
				speed_factor_min     = fminf(wheel_collision->speed_factor, speed_factor_min);
				rotation_factor_sum += wheel_collision->rot_factor;
				collision_count++;
			}
		}

		vehicle_collision_t* body_collision = &vehicle->body.collision;
		if (body_collision->valid)
		{
			speed_factor_min = fminf(body_collision->speed_factor, speed_factor_min);
			rotation_factor_sum += body_collision->rot_factor;
		}

		if (collision_count != 0 || body_collision->valid)
		{ 
			if (collision_count > 0 && vehicle->body.has_floor_collision)
				collision_count++; /* BUG only one collision counted */
			if (body_collision->valid)
				collision_count++;

			floor->speed_factor    = speed_factor_min;
			floor->rotation_factor = rotation_factor_sum / (float)collision_count;
		}
	}
}

void floor_update_sticky(floor_t* floor, vehicle_t* vehicle, kcl_t* kcl)
{
	if (vehicle->surface_props.has_sticky_road)
		floor->sticky_enabled = true;
	else if (!floor->sticky_enabled)
		return;

	physics_t* physics = &vehicle->physics;

	vec3_t pos, vel, down_vel;
	pos = physics->pos;
	vec3_mul(&physics->vel1_dir, physics->speed1, &vel);

	const float radius = 200.0f;
	down_vel = (vec3_t){ -radius * physics->mat.m01, -radius * physics->mat.m11, -radius * physics->mat.m21 };

	for (int i = 0; i < 3; i++)
	{
		hitbox_t hitbox;
		vec3_t hitbox_pos;
		collision_t collision;

		vec3_add(&pos, &vel, &hitbox_pos);
		hitbox_init(&hitbox, &hitbox_pos, NULL, radius, 0x400800);

		kcl_collision_hitbox(kcl, &hitbox, &collision);
		if (collision.surface_kinds & 0x400800)
		{
			vec3_t dir = physics->vel1_dir;
			vec3_cross_plane(&dir, &collision.floor_normal, &physics->vel1_dir);
			vec3_norm(&physics->vel1_dir);
			return;
		}

		vec3_add(&pos, &down_vel, &pos);
		vec3_mul(&vel, 0.5f, &vel);
	}

	floor->sticky_enabled = false;
}

void floor_activate_invincibility(floor_t* floor, uint16_t duration)
{
	floor->invincibility = max(floor->invincibility, duration);
}

bool floor_is_landing(floor_t* floor)
{
	return floor->airtime == 0 && floor->last_airtime != 0;
}