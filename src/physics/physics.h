#pragma once

#include "floor.h"

enum
{
	driving_dir_forward,
	driving_dir_braking,
	driving_dir_wait_backward,
	driving_dir_backward,
};

typedef struct vehicle_t vehicle_t;
typedef struct input_t input_t;
typedef struct bsp_t bsp_t;
typedef struct kmp_t kmp_t;
typedef struct kcl_t kcl_t;

typedef struct physics_t
{
	mat34_t		inv_inertia_tensor;
	float		rot_factor;
	mat34_t		mat;
	vec3_t		up;
	vec3_t		smoothed_up;
	vec3_t		dir;
	vec3_t		dir_diff;
	vec3_t		vel1_dir;
	bool		landing_dir_valid;
	float		landing_angle;
	vec3_t		landing_dir;
	vec3_t		pos;
	float		gravity;
	float		normal_acceleration;
	vec3_t		vel0;
	vec3_t		vel1;
	float		last_speed1;
	float		speed1;
	float		speed1_adj;
	float		speed1_soft_limit;
	float		speed1_ratio;
	vec3_t		vel;
	vec3_t		normal_rot_vec;
	vec3_t		rot_vec0;
	vec3_t		rot_vec2;
	quat_t		main_rot;
	quat_t		non_conserved_special_rot;
	quat_t		conserved_special_rot;
	quat_t		full_rot;
	float		stabilization_factor;
	int			driving_dir;
	int16_t		driving_backward_frame;
} physics_t;

void physics_init(physics_t* physics);
void physics_place(physics_t* physics, bsp_t* bsp, kmp_t* kmp, kcl_t* kcl);
void physics_update(physics_t* physics, vehicle_t* vehicle, int stage);
void physics_update_ups(physics_t* physics, vehicle_t* vehicle);
void physics_update_dirs(physics_t* physics, vehicle_t* vehicle);
void physics_update_accel(physics_t* physics, vehicle_t* vehicle, input_t* input, int stage);
void physics_update_rigid_body(physics_t* physics, vehicle_t* vehicle, vec3_t* pos, vec3_t* vel, vec3_t* nor);
void physics_update_landing_angle(physics_t* physics);
float physics_calc_acceleration(physics_t* physics, vehicle_t* vehicle);
void physics_stabilize(physics_t* physics, vehicle_t* vehicle);