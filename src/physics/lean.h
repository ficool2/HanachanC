#pragma once

typedef struct vehicle_t vehicle_t;

typedef struct
{
	float inc;
	float cap;
} lean_rot_params_t;

typedef struct
{
	lean_rot_params_t rot_params[3];
	float	rot_min;
	float	rot_max;
	float	stick_x_factor;
	float	s_factor;
} lean_params_t;

typedef struct
{
	float	rot;
	float	rot_diff;
	float	rot_cap;
	lean_params_t* params;
} lean_t;

void lean_init(lean_t* lean);
void lean_set_drift(lean_t* lean, bool inside_drift);
void lean_update(lean_t* lean, vehicle_t* vehicle, float stick_x, int stage);