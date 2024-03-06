#include "../common.h"

#include "ramp_boost.h"

void ramp_boost_init(ramp_boost_t* boost)
{
	boost->duration = 0;
}

void ramp_boost_update(ramp_boost_t* boost)
{
	if (boost->duration > 0)
		boost->duration--;
}