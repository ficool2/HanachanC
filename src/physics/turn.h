#pragma once

typedef struct vehicle_t vehicle_t;
typedef struct input_t input_t;

typedef struct
{
	float raw;
	float drift;
} turn_t;

void turn_init(turn_t* turn);
void turn_update(turn_t* update, vehicle_t* vehicle, float stick_x);
void turn_update_rot(turn_t* update, vehicle_t* vehicle, input_t* input);