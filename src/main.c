#include "SDL/SDL.h"
#include "tinydir.h"

#include <stdio.h>
#include <xmmintrin.h>

#include "common.h"
#include "fs/yaz.h"
#include "fs/arc.h"
#include "fs/rkg.h"
#include "fs/rkrd.h"
#include "course/course.h"
#include "player/player.h"
#include "physics/physics.h"
#include "vehicle/vehicle.h"
#include "game/game.h"

#include "graphics/graphics.h"

typedef struct
{
	const char* common_path;
	const char* course_path;
	const char* ghost_path;
	double		fps;
	double		frame_limit;
	uint32_t	frame_start;
	int			width;
	int			height;
	bool		cli;
	bool		start_paused;
} config_t;

void config_init(config_t* config)
{
	config->common_path       = NULL;
	config->course_path       = NULL;
	config->ghost_path	      = NULL;
	config->fps			      = 60.0;
	config->frame_limit       = 1000.0 / config->fps;
	config->frame_start       = 0;
	config->width             = 800;
	config->height            = 600;
	config->cli			      = false;
	config->start_paused      = false;
}

int main_graphics(game_t* game, config_t* config)
{
	if (!game_load_ghost(game, config->course_path, config->ghost_path))
	{
		printf("Failed to load ghost\n");
		return 1;
	}
	
	graphics_t* graphics = game->graphics;
	uint64_t last_time = SDL_GetPerformanceCounter();

	bool quit = false;
	while (!quit)
	{
		uint64_t current_time = SDL_GetPerformanceCounter();
		double elapsed_time = (current_time - last_time) * 1000 / (double)SDL_GetPerformanceFrequency();

		if (game->frame_idx < config->frame_start
			|| elapsed_time >= config->frame_limit)
		{
			graphics->frametime = elapsed_time;

			const uint8_t* key_state;
			int mouse_x, mouse_y;
			key_state = SDL_GetKeyboardState(NULL);
			SDL_GetRelativeMouseState(&mouse_x, &mouse_y);

			game_input(game, key_state, (float)mouse_x, (float)mouse_y);

			if (game->frame_idx < config->frame_start || !game->pause || game->step)
				game_simulate(game, config->frame_limit / 1000.0);

			if (game->frame_idx >= config->frame_start)
			{
				graphics_render(graphics, game);
				SDL_GL_SwapWindow(graphics->window);
			}

			last_time = current_time;

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					quit = true;
				}
			}
		}
	}

	game_unload_ghost(game);
	return 0;
}

void main_cli_run_ghost(game_t* game, config_t* config, const char* ghost_path)
{
	printf("Ghost: %s\n", ghost_path);

	if (!game_load_ghost(game, config->course_path, ghost_path))
	{
		printf("Failed to load ghost\n");
		return;
	}

	uint32_t frame;
	for (;;)
	{
		game_input(game, NULL, 0.0f, 0.0f);
		game_simulate(game, 1000.0 / config->fps);

		frame = game->frame_idx - 1;
		if (game->keyframes.frame_desync != UINT32_MAX || frame >= game->keyframes.frame_count)
			break;
	}

	uint32_t timer = ssub_uint32(frame, stage_frame_countdown);
	printf("Simulated %u/%u (in-game: %u) frames\n\n", frame, game->keyframes.frame_count, timer);

	game_unload_ghost(game);
}

int main_cli(game_t* game, config_t* config)
{
	uint64_t start_time = SDL_GetPerformanceCounter();

	tinydir_dir dir;
	tinydir_open(&dir, config->ghost_path);

	if (dir.has_next)
	{
		while (dir.has_next)
		{
			tinydir_file file;
			tinydir_readfile(&dir, &file);

			if (!stricmp(file.extension, "rkg"))
				main_cli_run_ghost(game, config, file.path);

			tinydir_next(&dir);
		}
	}
	else
	{
		main_cli_run_ghost(game, config, config->ghost_path);
	}

	double elapsed_time = (SDL_GetPerformanceCounter() - start_time) / (double)SDL_GetPerformanceFrequency();
	printf("Completed in %.2f seconds\n", elapsed_time);

	tinydir_close(&dir);
	return 0;
}

int main(int argc, char* argv[])
{
	int ret = 1;
	if (argc < 4)
	{
		printf(
			"usage: hanachanc <Common.szs> <course(s)> <ghost(s)> [parameters...]\n\n"
			" Parameter                 |  Default  | Description\n"
			"---------------------------+-----------+-------------------------------------\n"
			" -cli                      |    off    | Run simulation without graphics\n"
			" -pause                    |    off    | Start the simulation paused\n"
			" -fps            <float>   |    60     | Set the FPS limit\n"
			" -width          <int>     |    800    | Screen width\n"
			" -height         <int>     |    600    | Screen height\n"
			" -start          <int>     |    0      | Starting frame to simulate from\n"
			"---------------------------+-----------+--------------------------------------\n"
			"example: hanachanc Common.szs Course samples/bc64-rta-0-i.rkg -pause\n"
		);
		printf("\nPress a key to continue...\n");
		getchar();
		return ret;
	}

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

	config_t config;
	config_init(&config);

	config.common_path = argv[1];
	config.course_path = argv[2];
	config.ghost_path  = argv[3];

	if (argc > 4)
	{
		for (int i = 4; i < argc; i++)
		{
			if (!strcmp(argv[i], "-cli"))
			{
				config.cli = true;
			}
			else if (!strcmp(argv[i], "-pause"))
			{
				config.start_paused = true;
			}
			else if (!strcmp(argv[i], "-fps"))
			{
				if (argc <= i + 1)
				{
					printf("Missing parameter for -fps\n");
					return ret;
				}

				config.fps = atof(argv[++i]);
			}
			else if (!strcmp(argv[i], "-width"))
			{
				if (argc <= i + 1)
				{
					printf("Missing parameter for -width\n");
					return ret;
				}

				config.width = atoi(argv[++i]);
			}
			else if (!strcmp(argv[i], "-height"))
			{
				if (argc <= i + 1)
				{
					printf("Missing parameter for -height\n");
					return ret;
				}

				config.height = atoi(argv[++i]);
			}
			else if (!strcmp(argv[i], "-start"))
			{
				if (argc <= i + 1)
				{
					printf("Missing parameter for -start\n");
					return ret;
				}

				config.frame_start = (uint32_t)atoi(argv[++i]);
			}
			else
			{
				printf("Unknown parameter %s\n", argv[i]);
			}
		}
	}

	if (!config.cli)
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			printf("Failed to initialize SDL: %s\n", SDL_GetError());
			return ret;
		}
	}

	game_t game;
	game_init(&game);
	game.pause = config.start_paused;

	if (!game_load(&game, config.common_path))
		return ret;

	SDL_Window* window = NULL;
	SDL_GLContext context = NULL;
	graphics_t graphics;

	if (!config.cli)
	{
		window = SDL_CreateWindow("Ghost Replay", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.width, config.height, SDL_WINDOW_OPENGL);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_SetRelativeMouseMode(SDL_TRUE);
		context = SDL_GL_CreateContext(window);

		if (!graphics_init(&graphics, window, config.width, config.height))
		{
			game_free(&game);
			return ret;
		}
		game.graphics = &graphics;

		main_graphics(&game, &config);
	}
	else
	{
		main_cli(&game, &config);
	}

	game_free(&game);

	if (!config.cli)
	{
		graphics_free(&graphics);

		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
	}

	SDL_Quit();
	return 0;
}