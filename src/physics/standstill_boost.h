#pragma once

typedef struct vehicle_t vehicle_t;

typedef struct
{
	float	rotation;
	int16_t	charge;
} standstill_boost_t;

void standstill_boost_init  (standstill_boost_t* boost);
void standstill_boost_update(standstill_boost_t* boost, vehicle_t* vehicle, int stage);