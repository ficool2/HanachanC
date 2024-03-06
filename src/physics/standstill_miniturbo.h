#pragma once

typedef struct vehicle_t vehicle_t;
typedef struct input_t input_t;

typedef struct
{
	int16_t charge;
	bool	charging;
} standstill_miniturbo_t;

void standstill_miniturbo_init(standstill_miniturbo_t* ssmt);
void standstill_miniturbo_try_start(standstill_miniturbo_t* ssmt, vehicle_t* vehicle, input_t* input);
void standstill_miniturbo_update(standstill_miniturbo_t* ssmt, vehicle_t* vehicle);