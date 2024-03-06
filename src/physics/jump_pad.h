#pragma once

typedef struct physics_t physics_t;

typedef struct
{
	uint8_t	variant;
	bool	applied_dir;
} jump_pad_t;

void jump_pad_init(jump_pad_t* jump_pad);
void jump_pad_update(jump_pad_t* jump_pad, physics_t* physics, uint8_t new_variant);