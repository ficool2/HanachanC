#pragma once

typedef struct vehicle_t vehicle_t;

typedef struct
{
	float rot;
} dive_t;

void dive_init(dive_t* dive);
void dive_update(dive_t* dive, vehicle_t* vehicle, float stick_y);