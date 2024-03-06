#pragma once

typedef struct
{
	vec3_t pos;
	vec3_t angles;
} bikepart_handle_t;

typedef struct bikeparts_section_t
{
	float	camera_target_vertical_dist;
	float	tires_forward_rotation_speed;
	float	tires_backward_rotation_speed;
	bikepart_handle_t handle;
	float	unknown[13];
	int32_t	exhaust1_pipes_count;
	vec3_t	exhaust1_pos;
	vec2_t	exhaust1_rot;
	int32_t	exhaust2_pipes_count;
	vec3_t	exhaust2_pos;
	vec2_t	exhaust2_rot;
	vec3_t	ice_size;
	vec3_t	ice_pos;
	vec2_t	balloons_pos;
	vec2_t	unknown2;
} bikeparts_section_t;

typedef struct
{
	bikeparts_section_t* sections;
	uint32_t			 section_count;
} bikeparts_t;

extern parser_t bikeparts_parser;