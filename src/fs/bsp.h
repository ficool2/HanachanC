#pragma once

enum 
{ 
	bsp_max_wheels = 4,
	bsp_max_hitboxes = 16 
};

typedef struct bsp_hitbox_t
{
	uint16_t		enable;
	uint16_t		pad;
	vec3_t			sphere_center;
	float			sphere_radius;
	uint16_t		wall_only;
	uint16_t		tire_index;
} bsp_hitbox_t;

typedef struct bsp_wheel_t
{
	uint16_t		enable;
	uint16_t		pad;
	float			suspension_distance;
	float			suspension_speed;
	float			suspension_slack;
	vec3_t			suspension_top;
	float			rotation_x;
	float			radius;
	float			sphere_radius;
	uint32_t		unknown;
} bsp_wheel_t;

typedef struct bsp_t
{
	float			y_offset;
	bsp_hitbox_t	hitboxes[bsp_max_hitboxes];
	int				hitbox_count;
	vec3_t			cuboids[2];
	float			angular_velocity_boost;
	float			unknown;
	union
	{
		struct
		{
			bsp_wheel_t		wheel_front;
			bsp_wheel_t		wheel_back;
			bsp_wheel_t		unused[2];
		};

		bsp_wheel_t	wheels[4];
	};
	int				wheel_count;
	float			rumble_distance;
	float			rumble_speed;
} bsp_t;

extern parser_t bsp_parser;