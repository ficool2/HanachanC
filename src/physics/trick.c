#include "../common.h"
#include "../vehicle/vehicle.h"
#include "../fs/param.h"
#include "../player/player.h"

#include "trick.h"

void trick_init(trick_t* trick)
{
	trick->next_input			= trick_up;
	trick->next_timer			= 0;
	trick->boost_ramp_enabled	= false;
	trick->has_diving_rot_bonus	= false;
	trick->state				= trick_state_idle;
	memset(&trick->act, 0, sizeof(trick->act));
}

void trick_try_start(trick_t* trick, vehicle_t* vehicle)
{
	if (trick->state != trick_state_ready)
		return;

	physics_t* physics = &vehicle->physics;
	if (physics->speed1_ratio <= 0.5f)
	    return;

	trick_act_t* act = &trick->act;

	uint8_t boost_ramp_id = vehicle->surface_props.boost_ramp;
	switch (boost_ramp_id)
	{
		case 0:
		{
			trick_act_init_flip(act, trick->next_input, vehicle_is_bike(vehicle), true);
			break;
		}
		case 1:
		{
			trick_act_init_flip(act, trick->next_input, vehicle_is_bike(vehicle), false);
			break;
		}
		default:
		{
			trick_act_init_stunt(act, trick->next_input, vehicle_is_bike(vehicle));
			break;
		}
	}

	if (act->kind == trick_kind_stunt)
	{
		if (act->rot_dir != 0.0)
			trick->has_diving_rot_bonus = true;

		/* BUG game doesn't reset bonus */
	}
	else
	{
		trick->has_diving_rot_bonus = false;
	}

	if (vehicle->jump_pad.variant == variant_invalid)
	{
		int32_t weight_class = vehicle->stats.weight_class;

		vec3_t cross;
		vec3_cross(&physics->vel1_dir, &vec3_up, &cross);
		float norm = vec3_magu(&cross);
		float dot = vec3_dot(&physics->vel1_dir, &vec3_up);
		float angle = 90.0f - degreesf(fabsf(atan2f(norm, dot)));

		float dir_angle = trick_kind_dir_angle(act->kind, weight_class);
		float max_dir_angle_diff = trick_kind_max_dir_angle_diff(act->kind, weight_class);
		if (angle <= dir_angle)
		{
			float angle_diff;
			if (dir_angle < angle + max_dir_angle_diff)
				angle_diff = dir_angle - angle;
			else
				angle_diff = max_dir_angle_diff;

			mat34_t mat;
			vec3_t left, dir;

			dir = physics->dir;
			vec3_cross(&physics->smoothed_up, &physics->dir, &left);
			mat34_init_axis_angle(&mat, &left, -radiansf(angle_diff));
			mat34_mulv(&mat, &dir, &physics->dir);
			physics->vel1_dir = physics->dir;
		}
	}

	wheelie_cancel(&vehicle->wheelie);

	trick->state = trick_state_started;
}

void trick_update_rot(trick_t* trick, vehicle_t* vehicle)
{
	if (trick->state == trick_state_started)
	{
		trick_act_t* act = &trick->act;

		if (act->cooldown > 0)
			act->cooldown--;

		trick_act_update_rot(act);

		quat_t physics_rot = vehicle->physics.non_conserved_special_rot;
		quat_mulq(&physics_rot, &act->rot, &vehicle->physics.non_conserved_special_rot);
	}
}

void trick_update_next(trick_t* trick, vehicle_t* vehicle, uint8_t input)
{
	if (trick->state == trick_state_idle && input != trick_none)
	{
		trick->next_input = input;
		trick->next_timer = 15;
	}

	floor_t* floor = &vehicle->floor;
	if (trick_is_ready(trick, vehicle))
	{
		if (floor->airtime >= 3)
			trick->state = trick_state_ready;

		if (vehicle->ramp_boost.duration > 0)
			trick->boost_ramp_enabled = true;
	}
	else if (trick->next_timer > 0)
	{
		trick->next_timer--;
	}

	if (floor->airtime == 0 && !vehicle->surface_props.has_boost_ramp)
		trick->boost_ramp_enabled = false;
}

void trick_try_end(trick_t* trick, vehicle_t* vehicle)
{
	if (trick->state != trick_state_started)
		return;

	trick_act_t* act = &trick->act;
	if (act->cooldown > 0)
		return;

	quat_t physics_rot = vehicle->physics.conserved_special_rot;
	quat_mulq(&physics_rot, &act->rot, &vehicle->physics.conserved_special_rot);

	uint16_t duration = trick_kind_boost_duratiom(act->kind, vehicle_is_bike(vehicle));
	boost_activate(&vehicle->boost, boost_medium, duration);

	trick->state = trick_state_idle;
	trick->boost_ramp_enabled = false;
}

bool trick_is_ready(trick_t* trick, vehicle_t* vehicle)
{
	if (trick->next_timer == 0)
		return false;
	if (trick->state != trick_state_idle)
		return false;
	floor_t* floor = &vehicle->floor;
	if (floor->airtime == 0 || floor->airtime > 10)
		return false;
	return floor->has_trickable || vehicle->ramp_boost.duration > 0;
}


void trick_act_init_stunt(trick_act_t* act, uint8_t input, bool is_bike)
{
	act->kind = trick_kind_stunt;

	float rot_dir = 0.0f;
	switch (input)
	{
		case trick_left:
		{
			if (is_bike)
				rot_dir = 1.0f;
			break;
		}
		case trick_right:
		{
			if (is_bike)
				rot_dir = -1.0f;
			break;
		}
	}

	trick_act_create(act, rot_dir);
}

void trick_act_init_flip(trick_act_t* act, uint8_t input, bool is_bike, bool double_flip)
{
	act->kind = double_flip ? trick_kind_flip_double : trick_kind_flip;

	switch (input)
	{
		case trick_up:
		case trick_down:
		{
			act->flip_axis = is_bike ? axis_x : axis_z;
			break;
		}
		case trick_left:
		case trick_right:
		{
			act->flip_axis = axis_y;
			break;
		}
	}

	float rot_dir = 0.0f;
	switch (input)
	{
		case trick_down:
		case trick_left:
		{
			rot_dir = 1.0f;
			break;
		}
		case trick_up:
		case trick_right:
		{
			rot_dir = -1.0f;
			break;
		}
	}

	trick_act_create(act, rot_dir);
}

void trick_act_create(trick_act_t* act, float rot_dir)
{
	act->angle			= 0.0;
	act->angle_diff		= trick_kind_initial_angle_diff(act->kind);
	act->angle_diff_mul = 1.0f;
	act->rot_dir		= rot_dir;
	act->rot			= quat_identity;
	act->cooldown		= 5;
}

void trick_act_update_rot(trick_act_t* act)
{
	act->angle_diff     *= act->angle_diff_mul;
	act->angle_diff      = fmaxf(act->angle_diff, trick_kind_min_angle_diff(act->kind));
	
	act->angle_diff_mul -= trick_kind_angle_diff_mul_dec(act->kind);
	act->angle_diff_mul  = fmaxf(act->angle_diff_mul, trick_kind_min_angle_diff_mul(act->kind));
	
	act->angle          += act->angle_diff;
	act->angle           = fminf(act->angle, trick_kind_max_angle(act->kind));

	switch (act->kind)
	{
		case trick_kind_stunt:
		{
			if (act->rot_dir == 0.0f)
			{
				act->rot = quat_identity;
			}
			else
			{
				float rot_dir = act->rot_dir;
				float a = radiansf(20.0);
				float b = radiansf(60.0);
				float step = 256.0f / 360.0f;
				float s = sinf_inner(step * act->angle);
				vec3_t angles = (vec3_t){ -a * s, rot_dir * -b * s, rot_dir * a * s };
				quat_init_angles(&act->rot, &angles);
			}
			break;
		}
		case trick_kind_flip:
		case trick_kind_flip_double:
		{
			float angle = act->rot_dir * radiansf(act->angle);
			vec3_t angles = vec3_zero;
			angles.v[act->flip_axis] = angle;
			quat_init_angles(&act->rot, &angles);
			break;
		}
	}
}

float trick_kind_dir_angle(int trick_kind, int weight_class)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
		{
			switch (weight_class)
			{
				case driver_weight_light:
					return 40.0f;
				case driver_weight_medium:
					return 36.0f;
				case driver_weight_heavy:
					return 32.0f;
			}
		}
		case trick_kind_flip:
		case trick_kind_flip_double:
		{
			switch (weight_class)
			{
				case driver_weight_light:
					return 45.0f;
				case driver_weight_medium:
					return 42.0f;
				case driver_weight_heavy:
					return 39.0f;
			}
		}
	}

	UNREACHABLE();
}

float trick_kind_max_dir_angle_diff(int trick_kind, int weight_class)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
		{
			switch (weight_class)
			{
				case driver_weight_light:
					return 15.0f;
				case driver_weight_medium:
					return 13.0f;
				case driver_weight_heavy:
					return 11.0f;
			}
		}
		case trick_kind_flip:
		case trick_kind_flip_double:
		{
			switch (weight_class)
			{
				case driver_weight_light:
					return 20.0f;
				case driver_weight_medium:
					return 18.0f;
				case driver_weight_heavy:
					return 16.0f;
			}
		}
	}

	UNREACHABLE();
}

float trick_kind_max_angle(int trick_kind)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
			return 180.0f;
		case trick_kind_flip:
			return 360.0f;
		case trick_kind_flip_double:
			return 720.0f;
	}

	UNREACHABLE();
}

float trick_kind_initial_angle_diff(int trick_kind)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
			return 7.5f;
		case trick_kind_flip:
			return 11.0f;
		case trick_kind_flip_double:
			return 14.0f;
	}

	UNREACHABLE();
}

float trick_kind_min_angle_diff(int trick_kind)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
			return 2.5f;
		case trick_kind_flip:
		case trick_kind_flip_double:
			return 1.5f;
	}

	UNREACHABLE();
}

float trick_kind_min_angle_diff_mul(int trick_kind)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
			return 0.93f;
		case trick_kind_flip:
		case trick_kind_flip_double:
			return 0.9f;
	}

	UNREACHABLE();
}

float trick_kind_angle_diff_mul_dec(int trick_kind)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
			return 0.05f;
		case trick_kind_flip:
			return 0.0018f;
		case trick_kind_flip_double:
			return 0.0006f;
	}

	UNREACHABLE();
}

uint16_t trick_kind_boost_duratiom(int trick_kind, bool is_bike)
{
	switch (trick_kind)
	{
		case trick_kind_stunt:
			return is_bike ? 45 : 40;
		case trick_kind_flip:
			return is_bike ? 80 : 70;
		case trick_kind_flip_double:
			return is_bike ? 95 : 85;
	}

	UNREACHABLE();
}