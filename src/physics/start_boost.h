#pragma once

typedef struct
{
	float	charge;
} start_boost_t;

void	 start_boost_init  (start_boost_t* start_boost);
void	 start_boost_update(start_boost_t* start_boost, bool accelerate);
uint16_t start_boost_frames(start_boost_t* start_boost);