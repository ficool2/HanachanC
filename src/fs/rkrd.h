#pragma once

typedef struct physics_t physics_t;

typedef struct 
{
	vec3_t			rot_vec2;
	float			speed1_soft_limit;
	float			speed1;
	vec3_t			floor_nor;
	vec3_t			dir;
	vec3_t			pos;
	vec3_t			vel0;
	vec3_t			rot_vec0;
	vec3_t			vel2;
	vec3_t			vel;
	quat_t			main_rot;
	quat_t			full_rot;
	uint16_t		animation;
	uint16_t		checkpoint_idx;
} rkrd_frame_t;

typedef struct
{
	char			id[4];
	uint32_t		version;
} rkrd_header_t;

typedef struct rkrd_t
{
	rkrd_frame_t*	frames;
	uint32_t		frame_count;
	uint32_t		frame_desync;
} rkrd_t;

bool rkrd_check_desync(rkrd_frame_t* frame, physics_t* physics);

extern parser_t rkrd_parser;