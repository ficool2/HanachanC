#pragma once

typedef struct vehicle_t vehicle_t;
typedef struct boost_t boost_t;

enum
{
	drift_idle,
	drift_slipdrift,
	drift_hop,
	drift_normal,
};

typedef struct
{
	float	stick_x;
} drift_slipdrift_t;

typedef struct
{
	uint8_t frame;
	bool	in_stick;
	vec3_t  dir;
	vec3_t  up;
	float   stick_x;
	float	pos_y;
	float	vel_y;
	float	gravity;
} drift_hop_t;

void drift_hop_start(drift_hop_t* hop, vehicle_t* vehicle);
void drift_hop_update(drift_hop_t* hop, float stick_x);
void drift_hop_update_physics(drift_hop_t* hop);

typedef struct
{
	float	 stick_x;
	uint16_t mt_charge;
	uint16_t smt_charge;
	bool	 has_smt_charge;
} drift_normal_t;

void drift_normal_start(drift_normal_t* drift, vehicle_t* vehicle, float stick_x);
void drift_normal_update_miniturbo(drift_normal_t* drift, float stick_x);
void drift_normal_release_miniturbo(drift_normal_t* drift, boost_t* boost, uint16_t duration);

typedef struct
{
	float	angle;
	vec3_t	dir;
	float	bonus;
} drift_outside_t;

void drift_outside_update_angle_start(drift_outside_t* outside, drift_hop_t* hop, float stick_x, quat_t* rot0);
void drift_outside_update_angle_airborne(drift_outside_t* outside, quat_t* rot0);

typedef struct
{
	int		state;
	bool	has_outside_drift;
	bool	has_super_miniturbo;
	drift_outside_t	  outside;
	drift_slipdrift_t slipdrift;
	drift_hop_t		  hop;
	drift_normal_t    drift;
} drift_t;

void drift_init(drift_t* drift);
void drift_set_drift(drift_t* drift, bool inside_drift, bool super_miniturbo);
void drift_update(drift_t* drift, vehicle_t* vehicle, float stick_x, bool drift_input, bool last_drift_input);
void drift_update_hop(drift_t* drift, vehicle_t* vehicle);