#include "../common.h"
#include "../vehicle/vehicle.h"

#include "dive.h"

void dive_init(dive_t* dive)
{
	dive->rot = 0.0f;
}

void dive_update(dive_t* dive, vehicle_t* vehicle, float stick_y)
{
	floor_t* floor = &vehicle->floor;
	dive->rot *= 0.96f;

	if (floor->airtime == 0)
		return;

	float rot_diff = stick_y;
	if (vehicle->trick.state == trick_state_started && vehicle->trick.has_diving_rot_bonus)
		rot_diff = fminf(rot_diff + 0.4f, 1.0f);

	if (floor->airtime <= 50)
		rot_diff *= (float)floor->airtime / 50.0f;
	else if (fabs(rot_diff) < 0.1f)
		dive->rot -= (dive->rot + 0.025f) * 0.05f;

	physics_t* physics = &vehicle->physics;
	dive->rot = clampf(dive->rot + 0.005f * rot_diff, -0.8f, 0.8f);
	physics->rot_vec2.x += dive->rot;

	if (floor->airtime >= 50)
	{
		vec3_t up, cross;
		quat_rotate(&physics->main_rot, &vec3_up, &up);
		vec3_cross(&physics->up, &up, &cross);

		float norm = vec3_magu(&cross);
		float dot = vec3_dot(&physics->up, &up);
		float angle = degreesf(fabsf(atan2f(norm, dot))) - 20.0f;

		if (angle > 0.0) 
		{
			vec3_t v;
			float s = fminf(angle / 20.0f, 1.0f);
			quat_rotate(&physics->main_rot, &vec3_front, &v);
			if (v.y <= 0.0)
				physics->gravity *= 1.0f + 0.2f * s;
			else
				physics->gravity *= 1.0f - 0.2f * s;
		}
	}
}