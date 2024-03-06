#pragma once

typedef struct kcl_t kcl_t;
typedef struct vehicle_t vehicle_t;

typedef struct
{
	bool		valid;
	bool		has_trickable;
	bool		sticky_enabled;
	uint8_t		trickable_timer;
	vec3_t		normal;
	uint32_t	airtime;
	uint32_t	last_airtime;
	float		speed_factor;
	float		rotation_factor;
	uint16_t	invincibility;
} floor_t;

void floor_init(floor_t* floor);
void floor_update(floor_t* floor, vehicle_t* vehicle);
void floor_update_factors(floor_t* floor, vehicle_t* vehicle);
void floor_update_sticky(floor_t* floor, vehicle_t* vehicle, kcl_t* kcl);
void floor_activate_invincibility(floor_t* floor, uint16_t duration);
bool floor_is_landing(floor_t* floor);