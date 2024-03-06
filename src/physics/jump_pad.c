#include "../common.h"
#include "../fs/kcl.h"
#include "../physics/physics.h"

#include "jump_pad.h"

void jump_pad_init(jump_pad_t* jump_pad)
{
	jump_pad->variant	  = variant_invalid;
	jump_pad->applied_dir = false;
}

void jump_pad_update(jump_pad_t* jump_pad, physics_t* physics, uint8_t new_variant)
{
	jump_pad->applied_dir = false;

	if (jump_pad->variant != variant_invalid)
		return;
	if (new_variant == variant_invalid)
		return;

    physics->vel0.y   = variant_jump_pad_vel_y(new_variant);
    physics->normal_acceleration = 0.0f;

    vec3_t prev_dir   = physics->dir;
    physics->dir.y    = 0.0f;
    vec3_norm(&physics->dir);
    physics->vel1_dir = physics->dir;
    physics->speed1   *= vec3_dot(&physics->dir, &prev_dir);
    physics->speed1   = fmaxf(physics->speed1, variant_jump_pad_vel_y(new_variant));

    jump_pad->applied_dir = true;
    jump_pad->variant     = new_variant;
}