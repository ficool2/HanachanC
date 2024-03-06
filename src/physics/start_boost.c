#include "../common.h"

#include "start_boost.h"

void start_boost_init(start_boost_t* start_boost)
{
	start_boost->charge = 0.0f;
}

void start_boost_update(start_boost_t* start_boost, bool accelerate)
{
	if (accelerate)
		start_boost->charge += 0.02f - (0.02f - 0.002f) * start_boost->charge;
	else
		start_boost->charge *= 0.96f;

	start_boost->charge = clampf(start_boost->charge, 0.0f, 1.0f);
}

uint16_t start_boost_frames(start_boost_t* start_boost)
{
    float charge = start_boost->charge;
    if (charge <= 0.85)  return 0;
    if (charge <= 0.88)  return 10;
    if (charge <= 0.905) return 20;
    if (charge <= 0.925) return 30;
    if (charge <= 0.94)  return 45;
    if (charge <= 0.95)  return 70;

    /* TODO burnout */
    return 0;
}