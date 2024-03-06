#pragma once

typedef struct
{
	uint16_t	duration;
} ramp_boost_t;

void ramp_boost_init(ramp_boost_t* boost);
void ramp_boost_update(ramp_boost_t* boost);