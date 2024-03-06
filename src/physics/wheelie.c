#include "../common.h"
#include "../vehicle/vehicle.h"
#include "physics.h"
#include "trick.h"

#include "wheelie.h"

void wheelie_init(wheelie_t* wheelie)
{
	wheelie->is_wheelieing = false;
	wheelie->cooldown      = 0;
	wheelie->frame         = 0;
	wheelie->rot           = 0.0f;
	wheelie->rot_dec       = 0.0f;
}

void wheelie_update(wheelie_t* wheelie, vehicle_t* vehicle, uint8_t trick)
{
	physics_t* physics = &vehicle->physics;

	switch (trick)
	{
		case trick_up:
		{
			wheelie_try_start(wheelie, vehicle);
			break;
		}
		case trick_down:
		{
			wheelie_try_cancel(wheelie);
			break;
		}
	}

	if (wheelie->cooldown > 0)
		wheelie->cooldown--;

	if (wheelie->is_wheelieing)
	{
		wheelie->frame++;

		if (wheelie_should_cancel(wheelie, vehicle))
		{
			wheelie_cancel(wheelie);
		}
		else
		{
			wheelie->rot   = fminf(wheelie->rot + 0.01f, 0.07f);
			physics->rot_vec0.x *= 0.9f;
		}
	}
	else if (wheelie->rot > 0.0f)
	{
		wheelie->rot_dec  += 0.001f;
		wheelie->rot       = fmaxf(wheelie->rot - wheelie->rot_dec, 0.0f);
	}

	float c = vec3_dot(&vec3_up, &physics->vel1_dir);
	if (c <= 0.5f || wheelie->frame < 15)
		physics->rot_vec2.x -= wheelie->rot * (1.0f - fabsf(c));
}

void wheelie_start(wheelie_t* wheelie)
{
	wheelie->is_wheelieing = true;
	wheelie->frame         = 0;
	wheelie->cooldown      = 20;
}

void wheelie_cancel(wheelie_t* wheelie)
{
	wheelie->is_wheelieing = false;
	wheelie->rot_dec       = 0.0f;
}

void wheelie_try_start(wheelie_t* wheelie, vehicle_t* vehicle)
{
	if (wheelie->is_wheelieing || wheelie->cooldown > 0)
		return;
	if (vehicle->floor.airtime > 0)
		return;
	if (vehicle->drift.state == drift_hop || vehicle->drift.state == drift_normal)
		return;

	wheelie_start(wheelie);
}

void wheelie_try_cancel(wheelie_t* wheelie)
{
	if (!wheelie->is_wheelieing || wheelie->cooldown != 0)
		return;

	wheelie_cancel(wheelie);
	wheelie->cooldown = 20;
}

bool wheelie_should_cancel(wheelie_t* wheelie, vehicle_t* vehicle)
{
	if (wheelie->frame < 15)
		return false;
	if (wheelie->frame > 180)
		return true;

	physics_t* physics = &vehicle->physics;
	if (physics->speed1 < 0.0f)
		return true;
	return physics->speed1_ratio < 0.3f;
}