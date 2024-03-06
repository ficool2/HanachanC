#pragma once

typedef struct SDL_Window SDL_Window;
typedef struct GLTtext GLTtext;
typedef struct shader_t shader_t;
typedef struct shader_basic_t shader_basic_t;
typedef struct shader_basic_vcolor_t shader_basic_vcolor_t;
typedef struct game_t game_t;
typedef struct kcl_t kcl_t;
typedef struct hitbox_t hitbox_t;
typedef struct vehicle_t vehicle_t;

typedef struct
{
	vec3_t	pos;
	vec3_t	front;
	vec3_t	right;
	vec3_t	up;
	quat_t	quat;
	vec3_t	angles;
	float	fov;
	float   speed;
	float	sensitivity;
	mat44_t view;
	mat44_t proj;
	mat44_t viewproj;
	float	pos_lerp;
	float	rot_lerp;
} camera_t;

void camera_set_transform(camera_t* camera, const vec3_t* pos, const quat_t* quat);

typedef struct graphics_t
{
	SDL_Window*     window;
	int			    width;
	int			    height;
	float		    aspect;
	camera_t	    camera;
	GLTtext*	    text;
	vec3_t		    sun_dir;
	double		    frametime;
	unsigned int    vao;
	unsigned int    vbo;
	unsigned int    ebo;
	union
	{
		struct
		{
			shader_basic_t*        shader_basic;
			shader_basic_vcolor_t* shader_basic_vcolor;
		};

		shader_t* shaders[2];
	};
} graphics_t;

int  graphics_init(graphics_t* graphics, SDL_Window* window, int width, int height);
void graphics_free(graphics_t* graphics);
void graphics_render(graphics_t* graphics, game_t* game);
void graphics_draw_sphere(graphics_t* graphics, int rings, int sectors, const vec3_t* pos, const vec3_t* color, float radius);
void graphics_draw_kcl(graphics_t* graphics, kcl_t* kcl);
void graphics_draw_vehicle(graphics_t* graphics, vehicle_t* vehicle);
void graphics_draw_overlay(graphics_t* graphics, game_t* game);