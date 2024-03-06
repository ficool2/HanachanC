#include "../common.h"
#include "../vehicle/vehicle.h"
#include "../player/input.h"

#include "standstill_miniturbo.h"

void standstill_miniturbo_init(standstill_miniturbo_t* ssmt)
{
	ssmt->charge = 0;
	ssmt->charging = false;
}

void standstill_miniturbo_try_start(standstill_miniturbo_t* ssmt, vehicle_t* vehicle, input_t* input)
{
	if (fabsf(vehicle->physics.speed1) >= 10.0f
		|| !input->accelerate || !input->brake
		|| vehicle->ramp_boost.duration > 0
		|| boost_type(&vehicle->boost) != boost_none)
	{
		ssmt->charging = false;
	}
	else
	{
		ssmt->charging = true;
		vehicle->drift.state = drift_idle;
	}
}

void standstill_miniturbo_update(standstill_miniturbo_t* ssmt, vehicle_t* vehicle)
{
	if (!ssmt->charging)
	{
		if (ssmt->charge >= 75)
			boost_activate(&vehicle->boost, boost_weak, 30);

		ssmt->charge = 0;
	}
	else
	{
		ssmt->charge = min(ssmt->charge + 1, 75);
	}
}