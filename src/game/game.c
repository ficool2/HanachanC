#include "../common.h"
#include "../vehicle/vehicle.h"
#include "../course/course.h"
#include "../graphics/graphics.h"
#include "game.h"

#include "SDL/SDL.h"

void game_init(game_t* game)
{
	arc_parser.init(&game->common);
	course_parser.init(&game->course);

	param_parser.init(&game->kartparam);
	param_parser.init(&game->driverparam);
	bikeparts_parser.init(&game->bikeparts);

	rkg_parser.init(&game->ghost);
	rkrd_parser.init(&game->keyframes);

	game->players = NULL;
	game->player_count = 0;

	game->frame_idx = 0;
	game->frame_delta = 0.0;

	game->graphics = NULL;

	game->override_input = false;
	game->pause = false;
	game->step = false;
}

void game_free(game_t* game)
{
	arc_parser.free(&game->common);
	course_parser.free(&game->course);

	param_parser.free(&game->kartparam);
	param_parser.free(&game->driverparam);
	bikeparts_parser.free(&game->bikeparts);

	rkg_parser.free(&game->ghost);
	rkrd_parser.free(&game->keyframes);

	game_remove_players(game);
}

int	game_load(game_t* game, const char* common_path)
{
	int ret = 0;

	if (!parser_read(&arc_parser, &game->common, common_path))
	{
		printf("Couldn't parse %s\n", common_path);
		goto cleanup;
	}

	parser_pair_t files[] =
	{
		{ "kartParam.bin",			&game->kartparam,   &param_parser,     NULL },
		{ "driverParam.bin",		&game->driverparam, &param_parser,     NULL },
		{ "bikePartsDispParam.bin", &game->bikeparts,   &bikeparts_parser, NULL },
	};

	for (int i = 0; i < ARRAY_LEN(files); i++)
	{
		parser_pair_t* pair = &files[i];
		pair->bin = arc_find_data(&game->common, pair->filename);
		if (!pair->bin)
		{
			printf("Failed to find %s\n", pair->filename);
			goto cleanup;
		}

		if (!pair->parser->parse(pair->obj, pair->bin))
		{
			printf("Failed to parse %s\n", pair->filename);
			goto cleanup;
		}
	}

	ret = 1;

cleanup:
	if (ret != 1)
		game_free(game);

	return ret;
}

int game_load_course(game_t* game, const char* course_dir, uint8_t course_id)
{
	int ret = 0;
	const char* course_name = course_name_by_id(course_id);
	if (!course_name[0])
	{
		printf("Failed to load course, invalid ID %u\n", course_id);
		goto cleanup;
	}

	char course_path[_MAX_PATH];
	sprintf(course_path, "%s/%s.szs", course_dir, course_name);

	if (!parser_read(&course_parser, &game->course, course_path))
	{
		printf("Failed to load course %s, parsing error\n", course_name);
		goto cleanup;
	}

	ret = 1;

cleanup:
	if (ret != 1)
		course_parser.free(&game->course);

	return ret;
}

int game_load_ghost(game_t* game, const char* course_dir, const char* ghost_path)
{
	int ret = 0;
	player_t* player = NULL;

	if (!parser_read(&rkg_parser, &game->ghost, ghost_path))
		goto cleanup;

	strcpy(game->ghost.name, ghost_path);

	char keyframes_path[_MAX_PATH];
	strext(keyframes_path, ghost_path, "rkrd");

	if (!parser_read(&rkrd_parser, &game->keyframes, keyframes_path))
		goto cleanup;

	if (!game_load_course(game, course_dir, game->ghost.header.course_id))
		goto cleanup;

	player = malloc(sizeof(player_t));
	player_init(player);
	if (!player_load_ghost(player, game, &game->ghost))
		goto cleanup;

	game->frame_idx = 0;
	ret = 1;

cleanup:
	if (ret != 1)
	{
		if (player)
			player_free(player);
		rkrd_parser.free(&game->keyframes);
		rkg_parser.free(&game->ghost);
	}

	return ret;
}

void game_unload_ghost(game_t* game)
{
	course_parser.free(&game->course);

	rkg_parser.free(&game->ghost);
	rkrd_parser.free(&game->keyframes);

	game_remove_players(game);
}

void game_input(game_t* game, const uint8_t* key_state, float mouse_x, float mouse_y)
{
	player_t* player = game->players[0];
	memcpy(&player->input_last, &player->input, sizeof(player->input_last));

	if (game->override_input)
	{
		input_t* input = &game->players[0]->input;

		uint8_t trick = 0;
		if (key_state[SDL_SCANCODE_UP])
			trick = trick_up;
		else if (key_state[SDL_SCANCODE_DOWN])
			trick = trick_down;
		else if (key_state[SDL_SCANCODE_LEFT])
			trick = trick_left;
		else if (key_state[SDL_SCANCODE_RIGHT])
			trick = trick_up;

		float stick_x = 0.0f, stick_y = 0.0f;
		if (key_state[SDL_SCANCODE_W])
			stick_y = 1.0f;
		else if (key_state[SDL_SCANCODE_S])
			stick_y = -1.0f;

		if (key_state[SDL_SCANCODE_D])
			stick_x = 1.0f;
		else if (key_state[SDL_SCANCODE_A])
			stick_x = -1.0f;

		input->accelerate = stick_y > 0.0f;
		input->brake	  = stick_y < 0.0f;
		input->use_item   = key_state[SDL_SCANCODE_E] != 0;
		input->drift	  = key_state[SDL_SCANCODE_SPACE] != 0;
		input->stick_x	  = stick_x;
		input->stick_y    = stick_y;
		input->trick      = trick;
	}
	else if (game->ghost.frame_count != 0)
	{
		if (game->frame_idx >= stage_frame_countdown)
		{
			if (game->frame_idx < game->ghost.frame_count + stage_frame_countdown)
			{
				input_t* frame = &game->ghost.frames[game->frame_idx - stage_frame_countdown];
				memcpy(&player->input, frame, sizeof(player->input));
			}
		}
		else
		{
			memset(&player->input, 0, sizeof(player->input));
		}
	}

	if (key_state)
	{
		if (key_state[SDL_SCANCODE_Z] && !player->last_key_state[SDL_SCANCODE_Z])
			game->override_input = !game->override_input;

		if (key_state[SDL_SCANCODE_C] && !player->last_key_state[SDL_SCANCODE_C])
			game->pause = !game->pause;

		if (key_state[SDL_SCANCODE_X] && !player->last_key_state[SDL_SCANCODE_X])
			player->freecam = !player->freecam;

		if (game->pause && key_state[SDL_SCANCODE_RIGHT] && !player->last_key_state[SDL_SCANCODE_RIGHT])
			game->step = true;

		if (player->freecam)
		{
			camera_t* camera = &game->graphics->camera;

			vec3_t new_pos = camera->pos;
			quat_t new_quat = camera->quat;
			float speed = camera->speed;

			if (key_state[SDL_SCANCODE_LSHIFT])
				speed *= 3.0f;

			if (key_state[SDL_SCANCODE_W])
				vec3_muladd(&new_pos, -speed, &camera->front, &new_pos);
			if (key_state[SDL_SCANCODE_S])
				vec3_muladd(&new_pos, speed, &camera->front, &new_pos);
			if (key_state[SDL_SCANCODE_A])
				vec3_muladd(&new_pos, -speed, &camera->right, &new_pos);
			if (key_state[SDL_SCANCODE_D])
				vec3_muladd(&new_pos, speed, &camera->right, &new_pos);
			if (key_state[SDL_SCANCODE_SPACE])
				vec3_muladd(&new_pos, -speed, &camera->up, &new_pos);

			if (mouse_x != 0.0f || mouse_y != 0.0f)
			{
				vec3_t new_ang = camera->angles;

				new_ang.x += mouse_y * camera->sensitivity;
				new_ang.y -= mouse_x * camera->sensitivity;

				new_ang.y = anglenormf(new_ang.y);
				new_ang.x = clampf(new_ang.x, -90.0f, 90.0f);

				camera->angles = new_ang;

				vec3_radians(&new_ang, &new_ang);
				quat_init_angles(&new_quat, &new_ang);

				vec3_t end_ang;
				quat_angles(&new_quat, &end_ang);
			}

			camera_set_transform(camera, &new_pos, &new_quat);
		}

		memcpy(&player->last_key_state, key_state, sizeof(player->last_key_state));
	}
}

void game_simulate(game_t* game, double deltatime)
{
	game->frame_delta = deltatime;

	for (int i = 0; i < game->player_count; i++)
	{
		player_t* player = game->players[i];
		player_update(player, game);
	}

	uint32_t prev_frame_idx = game->frame_idx++;

	if (game->ghost.frame_count != 0)
	{
		if (game->keyframes.frame_desync == UINT32_MAX 
			&& prev_frame_idx < game->keyframes.frame_count)
		{
			rkrd_frame_t* rkrd_frame = &game->keyframes.frames[prev_frame_idx];
			player_t* player = game->players[0];

			if (!rkrd_check_desync(rkrd_frame, &player->vehicle->physics))
				game->keyframes.frame_desync = prev_frame_idx;
		}
	}

	game->step = false;
}

void game_add_player(game_t* game, player_t* player)
{
	game->player_count++;
	game->players = realloc(game->players, game->player_count * sizeof(*game->players));
	game->players[game->player_count - 1] = player;
}

void game_remove_players(game_t* game)
{
	for (int i = 0; i < game->player_count; i++)
		player_free(game->players[i]);
	free(game->players);
	game->players = NULL;
	game->player_count = 0;
}

int game_get_stage(game_t* game)
{
	if (game->frame_idx >= 0 && game->frame_idx <= stage_frame_countdown - 1)
		return stage_intro;
	if (game->frame_idx >= stage_frame_countdown && game->frame_idx <= stage_frame_race)
		return stage_countdown;
	return stage_race;
}