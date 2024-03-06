#pragma once

#include "input.h"
#include "SDL/SDL_scancode.h"

typedef struct game_t game_t;
typedef struct course_t course_t;
typedef struct vehicle_t vehicle_t;
typedef struct rkg_t rkg_t;

typedef struct player_t
{
	vehicle_t*				vehicle;
	input_t					input;
	input_t					input_last;
	uint8_t					last_key_state[SDL_NUM_SCANCODES];
	bool					freecam;
} player_t;

void player_init(player_t* player);
void player_free(player_t* player);
int  player_load_ghost(player_t* player, game_t* game, rkg_t* rkg);
void player_update(player_t* player, game_t* game);