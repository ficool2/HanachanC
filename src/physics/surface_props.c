#include "../common.h"
#include "../fs/kcl.h"

#include "surface_props.h"

void surface_props_init(surface_props_t* surface_props)
{
	surface_props->has_boost_panel   = false;
	surface_props->has_boost_ramp    = false;
	surface_props->has_sticky_road   = false;
	surface_props->has_non_trickable = false;
	surface_props->boost_ramp        = variant_invalid;
	surface_props->jump_pad          = variant_invalid;
}

void surface_props_add(surface_props_t* surface_props, collision_t* collision, bool allow_boost_panels)
{
	/* TODO label these masks */
	if (collision_find_furthest(collision, 0x800))
		surface_props->has_sticky_road = false;

	if (collision_find_furthest(collision, 0x20E80FFF))
	{
		if (allow_boost_panels && (collision->surface_kinds & 0x40))
			surface_props->has_boost_panel = true;

		hit_t* hit = (collision->surface_kinds & 0x80) ? collision_find_furthest(collision, 0x80) : NULL;
		if (hit)
		{
			surface_props->has_boost_ramp = true;
			surface_props->boost_ramp = hit->surface >> 5 & 7;
		}
		else
		{
			surface_props->has_boost_ramp = false;
			surface_props->boost_ramp = variant_invalid;
			surface_props->has_non_trickable = true; /* TODO: redundant? */
		}

		if (collision->surface_kinds & 0x400000)
			surface_props->has_sticky_road = true;

		hit = collision_find_furthest(collision, 0x100);
		if (hit)
			surface_props->jump_pad = hit->surface >> 5 & 7;
	}
}

void surface_props_reset(surface_props_t* surface_props)
{
	surface_props->has_boost_panel	 = false;
	surface_props->has_boost_ramp	 = false;
	surface_props->has_sticky_road	 = false;
	surface_props->has_non_trickable = false;
}