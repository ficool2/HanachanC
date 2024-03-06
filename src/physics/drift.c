#include "../common.h"
#include "../vehicle/vehicle.h"
#include "boost.h"

#include "drift.h"

void drift_hop_start(drift_hop_t* hop, vehicle_t* vehicle)
{
	quat_t* rot = &vehicle->physics.main_rot;

	wheelie_cancel(&vehicle->wheelie);
	vehicle->physics.vel0.y = 10.0f;
	vehicle->physics.normal_acceleration = 0.0f;
	vehicle->drift.state = drift_hop;

	hop->frame     = 0;
	quat_rotate(rot, &vec3_front, &hop->dir);
	quat_rotate(rot, &vec3_up, &hop->up);
	hop->stick_x  = 0.0f;
	hop->pos_y    = 0.0f;
	hop->vel_y    = 10.0f;
	hop->gravity  = vehicle->physics.gravity;
	hop->in_stick = false;
}

void drift_hop_update(drift_hop_t* hop, float stick_x)
{
	hop->frame = min(hop->frame + 1, 3);
	
	if (!hop->in_stick && stick_x != 0.0f)
	{
		hop->in_stick = true;
		hop->stick_x  = copysignf(1.0f, stick_x) * ceilf(fabsf(stick_x));
	}
}

void drift_hop_update_physics(drift_hop_t* hop)
{
	hop->vel_y *= 0.998f;
	hop->vel_y += hop->gravity;
	hop->pos_y += hop->vel_y;

	if (hop->pos_y < 0.0f)
	{
		hop->vel_y = 0.0f;
		hop->pos_y = 0.0f;
	}
}


void drift_normal_start(drift_normal_t* drift, vehicle_t* vehicle, float stick_x)
{
	if (vehicle->drift.has_outside_drift)
		vehicle->drift.outside.bonus = vehicle->physics.speed1_ratio * vehicle->stats.drift_tightness_manual * 0.5f;

	drift->stick_x        = stick_x;
	drift->mt_charge      = 0;
	drift->smt_charge     = 0;
	drift->has_smt_charge = !vehicle_is_bike(vehicle);
	vehicle->drift.state  = drift_normal;
}

void drift_normal_update_miniturbo(drift_normal_t* drift, float stick_x)
{
	uint16_t charge_inc = stick_x * drift->stick_x > 0.4f ? 5 : 2;
	uint16_t charge = drift->mt_charge + charge_inc;
	drift->mt_charge = min(charge, 271);

	if (drift->has_smt_charge)
	{
		if (drift->mt_charge >= 271)
		{
			charge = drift->smt_charge + charge_inc;
			drift->smt_charge = min(charge, 301);
		}
	}
}

void drift_normal_release_miniturbo(drift_normal_t* drift, boost_t* boost, uint16_t duration)
{
	if (drift->has_smt_charge && drift->smt_charge >= 301)
		boost_activate(boost, boost_weak, duration * 3);
	else if (drift->mt_charge >= 271)
		boost_activate(boost, boost_weak, duration);
}

void drift_outside_update_angle_start(drift_outside_t* outside, drift_hop_t* hop, float stick_x, quat_t* rot0)
{
	vec3_t front, cross, rej;
	quat_rotate(rot0, &vec3_front, &front);
	vec3_rej_unit(&front, &hop->up, &rej);
	float sq_norm = vec3_magsqr(&rej);
	if (sq_norm > FLT_EPSILON)
	{
		vec3_norm(&rej);
		vec3_cross(&hop->dir, &rej, &cross);
		float norm = vec3_magu(&cross);
		float dot = vec3_dot(&hop->dir, &rej);
		float angle_diff = degreesf(atan2f(norm, dot)) * stick_x;
		outside->angle = clampf(outside->angle + angle_diff, -60.0f, 60.0f);
	}
}

void drift_outside_update_angle_airborne(drift_outside_t* outside, quat_t* rot0)
{
	vec3_t up, rej, dir, cross;
	quat_rotate(rot0, &vec3_up, &up);
	vec3_rej_unit(&outside->dir, &up, &rej);
	float sq_norm = vec3_magsqr(&rej);
	if (sq_norm > FLT_EPSILON)
	{
		vec3_norm(&rej);
		quat_rotate(rot0, &vec3_front, &dir);
		vec3_cross(&rej, &dir, &cross);
		float norm = vec3_magu(&cross);
		float dot = vec3_dot(&rej, &dir);
		float angle_diff = degreesf(atan2f(norm, dot));
		float sign = copysignf(1.0f, dir.x * (dir.z - rej.z) - dir.z * (dir.x - rej.x));
		outside->angle += sign * angle_diff;
	}
}


void drift_init(drift_t* drift) 
{
	drift->state               = drift_idle;
	drift->has_outside_drift   = false;
	drift->has_super_miniturbo = false;

	memset(&drift->outside,   0, sizeof(drift->outside));
	memset(&drift->slipdrift, 0, sizeof(drift->slipdrift));
	memset(&drift->hop,       0, sizeof(drift->hop));
	memset(&drift->drift,     0, sizeof(drift->drift));

	drift->outside.dir = vec3_back;
}

void drift_set_drift(drift_t* drift, bool inside_drift, bool super_miniturbo)
{
	drift->has_outside_drift   = !inside_drift;
	drift->has_super_miniturbo = super_miniturbo;
}

void drift_update(drift_t* drift, vehicle_t* vehicle, float stick_x, bool drift_input, bool drift_last_input)
{
	physics_t* physics = &vehicle->physics;
	floor_t* floor = &vehicle->floor;
	bool grounded = floor->airtime == 0;

	if (!grounded && stick_x != 0.0f)
	{
		if (drift_input && drift->state == drift_idle)
		{
			wheelie_cancel(&vehicle->wheelie);
			drift->slipdrift.stick_x = copysignf(1.0f, stick_x);
			drift->state = drift_slipdrift;
		}
		else if (!drift_input && drift->state == drift_slipdrift)
		{
			drift->state = drift_idle;
		}
	}

	switch (drift->state)
	{
		case drift_idle:
		{
			if (grounded && drift_input && !drift_last_input)
				drift_hop_start(&drift->hop, vehicle);
			break;
		}
		case drift_slipdrift:
		{
			if (grounded)
			{
				drift_normal_start(&drift->drift, vehicle, drift->slipdrift.stick_x);
			}
			else if (floor->airtime > 5)
			{
				if (drift->has_outside_drift)
					drift_outside_update_angle_airborne(&drift->outside, &physics->main_rot);
			}
			break;
		}
		case drift_hop:
		{
			drift_hop_update(&drift->hop, stick_x);

			float hop_stick_x;
			if (drift->state == drift_hop && drift->hop.in_stick)
				hop_stick_x = drift->hop.stick_x;
			else
				hop_stick_x = 0.0f;

			if (drift->hop.frame >= 3 && grounded)
			{
				if (drift->has_outside_drift)
					drift_outside_update_angle_start(&drift->outside, &drift->hop, hop_stick_x, &physics->main_rot);

				if (drift->state == drift_hop && drift->hop.in_stick && drift_input)
					drift_normal_start(&drift->drift, vehicle, hop_stick_x);
				else
					drift->state = drift_idle;
			}
			else if (drift->hop.frame < 3 && drift_input && !drift_last_input)
			{
				drift_hop_start(&drift->hop, vehicle);
			}
			break;
		}
		case drift_normal:
		{
			if (floor->airtime > 5)
			{
				if (drift->has_outside_drift)
					drift_outside_update_angle_airborne(&drift->outside, &physics->main_rot);
			}
			break;
		}
	}

	if (drift->has_outside_drift)
		quat_rotate(&physics->main_rot, &vec3_front, &drift->outside.dir);

	if (drift->state == drift_idle && grounded)
	{
		if (drift->has_outside_drift)
		{
			float dec = vehicle->stats.drift_decrement;
			float angle = drift->outside.angle;
			drift->outside.angle = copysignf(1.0f, angle) * fmaxf(fabsf(angle) - dec, 0.0f);
		}
	}
	else if (drift->state == drift_normal)
	{
		if (drift_input)
		{
			drift->outside.bonus *= 0.99f;

			if (floor->airtime <= 5)
			{
				if (drift->has_outside_drift)
				{
					float last_angle = drift->outside.angle * drift->drift.stick_x;
					float target_angle = vehicle->stats.drift_target_angle;
					float next_angle;
					if (last_angle < target_angle)
						next_angle = fminf(last_angle + 150.0f * vehicle->stats.drift_tightness_manual, target_angle);
					else if (last_angle > target_angle)
						next_angle = fmaxf(last_angle - 2.0f, target_angle);
					else
						next_angle = last_angle;	

					drift->outside.angle = next_angle * drift->drift.stick_x;
				}

				drift_normal_update_miniturbo(&drift->drift, stick_x);
			}
		}
		else
		{
			drift_normal_release_miniturbo(&drift->drift, &vehicle->boost, (uint16_t)vehicle->stats.mini_turbo_duration);
			drift->state = drift_idle;
		}
	}
}