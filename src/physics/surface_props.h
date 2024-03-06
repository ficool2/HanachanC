#pragma once

typedef struct
{
	bool	has_boost_panel;
	bool	has_boost_ramp;
	bool	has_sticky_road;
	bool	has_non_trickable;
	uint8_t	boost_ramp;
	uint8_t	jump_pad;
} surface_props_t;

void surface_props_init (surface_props_t* surface_props);
void surface_props_add  (surface_props_t* surface_props, collision_t* collision, bool allow_boost_panels);
void surface_props_reset(surface_props_t* surface_props);