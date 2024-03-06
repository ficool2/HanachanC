#pragma once

typedef struct physics_t physics_t;

typedef struct
{
	bool is_wheelieing;
	uint16_t cooldown;
	uint16_t frame;
	float rot;
	float rot_dec;
} wheelie_t;

void wheelie_init  (wheelie_t* wheelie);
void wheelie_update(wheelie_t* wheelie, vehicle_t* vehicle, uint8_t trick);
void wheelie_start (wheelie_t* wheelie);
void wheelie_cancel(wheelie_t* wheelie);
void wheelie_try_start(wheelie_t* wheelie, vehicle_t* vehicle);
void wheelie_try_cancel(wheelie_t* wheelie);
bool wheelie_should_cancel(wheelie_t* wheelie, vehicle_t* vehicle);