#pragma once

#include "../fs/arc.h"
#include "../fs/param.h"
#include "../fs/bikeparts.h"
#include "../fs/rkg.h"
#include "../fs/rkrd.h"
#include "../player/player.h"
#include "../course/course.h"

typedef struct graphics_t graphics_t;

enum
{
	stage_intro,
	stage_countdown,
	stage_race
};

enum
{
	stage_frame_intro     = 0,
	stage_frame_countdown = 172,
	stage_frame_race      = 410,
};

typedef struct game_t
{
	arc_t			common;
	course_t		course;

	param_t			kartparam;
	param_t			driverparam;
	bikeparts_t		bikeparts;

	player_t**		players;
	int				player_count;

	uint32_t		frame_idx;
	double			frame_delta;

	rkg_t			ghost;
	rkrd_t			keyframes;

	graphics_t*		graphics;

	bool			override_input;
	bool			pause;
	bool			step;
} game_t;

void game_init(game_t* game);
void game_free(game_t* game);
int	 game_load(game_t* game, const char* common_path);
int  game_load_course(game_t* game, const char* course_dir, uint8_t course_id);
int  game_load_ghost(game_t* game, const char* course_dir, const char* ghost_path);
void game_unload_ghost(game_t* game);
void game_input(game_t* game, const uint8_t* key_state, float mouse_x, float mouse_y);
void game_simulate(game_t* game, double deltatime);

void game_add_player(game_t* game, player_t* player);
void game_remove_players(game_t* game);
int  game_get_stage(game_t* game);