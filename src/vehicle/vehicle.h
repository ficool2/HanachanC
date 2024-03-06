#pragma once

#include "../fs/param.h"
#include "../fs/bikeparts.h"
#include "../fs/bsp.h"
#include "../fs/kcl.h"

#include "../physics/physics.h"
#include "../physics/surface_props.h"
#include "../physics/turn.h"
#include "../physics/drift.h"
#include "../physics/wheelie.h"
#include "../physics/lean.h"
#include "../physics/dive.h"
#include "../physics/start_boost.h"
#include "../physics/standstill_boost.h"
#include "../physics/standstill_miniturbo.h"
#include "../physics/ramp_boost.h"
#include "../physics/boost.h"
#include "../physics/trick.h"
#include "../physics/jump_pad.h"

typedef struct game_t game_t;
typedef struct player_t player_t;
typedef struct course_t course_t;

enum
{
	vehicle_kart_id = 0,
	vehicle_bike_id = 18,
	vehicle_max_id  = 36,
};

enum
{
	drift_kart_outside,
	drift_bike_outside,
	drift_bike_inside,
};

/* TODO separate these into their own files */

typedef struct
{
	bool	valid;
	bool	has_trickable;
	uint8_t	count;
	vec3_t	floor_normal;
	float	speed_factor;
	float	rot_factor;
} vehicle_collision_t;

void vehicle_collision_init(vehicle_collision_t* collision);
void vehicle_collision_add(vehicle_collision_t* collision, vehicle_t* vehicle, collision_t* kcl_collision);
void vehicle_collision_set_normal(vehicle_collision_t* collision, vec3_t* normal);
void vehicle_collision_finalize(vehicle_collision_t* collision);

typedef struct
{
	bikepart_handle_t*		handle;
	bsp_wheel_t				bsp;
	vec3_t					axis;
	float					axis_s;
	vec3_t					topmost_pos;
	vec3_t					pos;
	vec3_t					last_pos;
	vec3_t					last_pos_rel;
	hitbox_t				hitbox;
	vec3_t					hitbox_pos_rel;
	vehicle_collision_t		collision;
} vehicle_wheel_t;

void vehicle_wheel_init(vehicle_wheel_t* wheel, bsp_wheel_t* bsp, bikepart_handle_t* handle, vec3_t* pos, int index);
bool vehicle_wheel_update(vehicle_wheel_t* wheel, vehicle_t* vehicle, kcl_t* kcl, vec3_t* out);
void vehicle_wheel_update_suspension(vehicle_wheel_t* wheel, vehicle_t* vehicle, vec3_t* movement);
void vehicle_wheel_mat(vehicle_wheel_t* wheel, physics_t* physics, mat34_t* mat);
void vehicle_wheel_dump_state(vehicle_wheel_t* wheel);

typedef struct
{
	hitbox_t*				hitboxes;
	bsp_hitbox_t*			bsp_hitboxes;
	int						hitbox_count;
	vehicle_collision_t		collision;
	bool					has_floor_collision;
} vehicle_body_t;

void vehicle_body_init(vehicle_body_t* body, bsp_t* bsp, mat34_t* mat);
void vehicle_body_free(vehicle_body_t* body);
void vehicle_body_update(vehicle_body_t* body, vehicle_t* vehicle, kcl_t* kcl);

typedef struct vehicle_t
{
	uint8_t					id;

	param_section_t			stats;
	bikeparts_section_t*	bikeparts;
	bikepart_handle_t*		handle;
	player_t*				player;
	bsp_t*					bsp;

	physics_t				physics;
	floor_t					floor;
	surface_props_t			surface_props;

	turn_t					turn;
	drift_t					drift;
	wheelie_t				wheelie;
	lean_t					lean;
	dive_t					dive;

	vehicle_body_t			body;
	vehicle_wheel_t			wheels[bsp_max_wheels];
	int						wheel_count;

	start_boost_t			start_boost;
	standstill_boost_t		standstill_boost;
	standstill_miniturbo_t  standstill_miniturbo;
	ramp_boost_t			ramp_boost;
	boost_t					boost;

	trick_t					trick;
	jump_pad_t				jump_pad;
} vehicle_t;

void		vehicle_init(vehicle_t* vehicle, bsp_t* bsp, game_t* game, uint8_t vehicle_id, uint8_t character_id);
void		vehicle_free(vehicle_t* vehicle);
vehicle_t*	vehicle_load(player_t* player, game_t* game, uint8_t vehicle_id, uint8_t character_id);
void        vehicle_place(vehicle_t* vehicle, course_t* course);
const char*	vehicle_name_by_id(uint8_t id);

inline bool vehicle_is_bike(vehicle_t* vehicle)
{
	int32_t drift_type = vehicle->stats.drift_type;
	return drift_type == drift_bike_outside || drift_type == drift_bike_inside;
}

inline bool vehicle_is_inside_drift(vehicle_t* vehicle)
{
	int32_t drift_type = vehicle->stats.drift_type;
	return drift_type == drift_bike_inside;
}