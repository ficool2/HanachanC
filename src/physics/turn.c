#include "../common.h"
#include "../vehicle/vehicle.h"
#include "../player/input.h"

#include "turn.h"

void turn_init(turn_t* turn)
{
	turn->raw   = 0.0f;
	turn->drift = 0.0f;
}

void turn_update(turn_t* turn, vehicle_t* vehicle, float stick_x)
{
	if (vehicle->drift.state == drift_hop && vehicle->drift.hop.in_stick)
		stick_x = vehicle->drift.hop.stick_x;
	else if (vehicle->floor.airtime > 20)
		stick_x *= 0.01f;

	float reactivity;
	if (vehicle->drift.state == drift_normal)
		reactivity = vehicle->stats.drift_reactivity;
	else
		reactivity = vehicle->stats.handling_reactivity;

	turn->raw = (1.0f - reactivity) * turn->raw + reactivity * -stick_x;

	if (vehicle->drift.state == drift_normal)
	{
		float drift_stick_x = vehicle->drift.drift.stick_x;
		float drift_turn = 0.5f * (turn->raw - drift_stick_x);
		turn->drift = (drift_turn * 0.8f - drift_stick_x * 0.2f);
		turn->drift = clampf(turn->drift, -1.0f, 1.0f);
	}
	else
	{
		turn->drift = turn->raw;
	}
}

void turn_update_rot(turn_t* turn, vehicle_t* vehicle, input_t* input)
{
	physics_t* physics = &vehicle->physics;
	float rot;

	if (!vehicle->standstill_miniturbo.charging)
	{
		if (vehicle->drift.state == drift_normal)
			rot = turn->drift * (vehicle->stats.drift_tightness_manual + vehicle->drift.outside.bonus);
		else
			rot = turn->drift * vehicle->stats.handling_tightness_manual;

		if (vehicle->drift.state == drift_hop && vehicle->drift.hop.pos_y > 0.0f)
			rot *= 1.4f;

		if (vehicle->drift.state != drift_normal)
		{
			if (!input->brake || physics->speed1 > 0.0f)
			{
				if (fabsf(physics->speed1) < 1.0f)
					rot = 0.0f;
				else if (physics->speed1 < 20.0f)
					rot = 0.4f * rot + (physics->speed1 / 20.0f) * (rot * 0.6f);
				else if (physics->speed1 < 70.0)
					rot = 0.5f * rot + (1.0f - (physics->speed1 - 20.0f) / (70.0f - 20.0f)) * (rot * 0.5f);
				else
					rot = 0.5f * rot;
			}
			else
			{
				rot = -rot;
			}
		}

		uint32_t airtime = vehicle->floor.airtime;
		if (airtime > 0 && vehicle->ramp_boost.duration > 0 && vehicle->trick.boost_ramp_enabled)
			rot = 0.0f;
		else if (airtime > 70)
			rot = 0.0f;
		else if (airtime >= 30)
			rot = fmaxf((1.0f - 0.025f * (float)(airtime - 30)) * rot, 0.0f);
	}
	else
	{
		rot = turn->drift * 0.04f;
	}

	vec3_t front, cross;
	quat_rotate(&physics->main_rot, &vec3_front, &front);
	vec3_cross(&front, &physics->dir, &cross);
	float norm = vec3_magu(&cross);
	float dot = vec3_dot(&front, &physics->dir);
	float angle = degreesf(fabsf(atan2f(norm, dot)));
	if (angle > 60.0f)
		rot *= fmaxf(1.0f - (angle - 60.0f) / (100.0f - 60.0f), 0.0f);

	if (vehicle->wheelie.is_wheelieing)
		rot *= 0.2f;

	physics->rot_vec2.y += rot;
}