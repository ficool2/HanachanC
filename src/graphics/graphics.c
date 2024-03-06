#include "../common.h"

#define GLT_IMPLEMENTATION
#define GLT_MANUAL_VIEWPORT
#include "GL/glew.h"
#include "GL/gltext.h"

#include "../game/game.h"
#include "../course/course.h"
#include "../vehicle/vehicle.h"
#include "../fs/kcl.h"

#include "shader.h"
#include "graphics.h"

#include "shader_basic.h"
#include "shader_basic_vcolor.h"

STATIC_ASSERT(STRUCT_ARRAY_LEN(graphics_t, shaders) == shader_type_last);

void camera_set_transform(camera_t* camera, const vec3_t* position, const quat_t* quat)
{
	camera->pos  = *position;
	camera->quat = *quat;

	quat_rotate(&camera->quat, &vec3_front, &camera->front);
	quat_rotate(&camera->quat, &vec3_right, &camera->right);
	quat_rotate(&camera->quat, &vec3_up,    &camera->up);
}

int graphics_init(graphics_t* graphics, SDL_Window* window, int width, int height)
{
	graphics->window = window;
	memset(graphics->shaders, 0, sizeof(graphics->shaders));

	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		printf("Failed to initialize OpenGL: %s\n", glewGetErrorString(error));
		return 0;
	}

	if (!shader_compile_all(&graphics->shaders[0]))
		return 0;

	glGenVertexArrays(1, &graphics->vao);
	glGenBuffers(1, &graphics->vbo);
	glGenBuffers(1, &graphics->ebo);

	graphics->width = width;
	graphics->height = height;
	graphics->aspect = (float)width / (float)height;

	graphics->sun_dir = (vec3_t){ 0.2f, 0.8f, 0.2f };
	vec3_norm(&graphics->sun_dir);

	graphics->frametime = 0.1;

	gltInit();
	graphics->text = gltCreateText();
	gltViewport(width, height);

	camera_t* camera = &graphics->camera;
	camera_set_transform(camera, &vec3_zero, &quat_identity);

	camera->angles = vec3_zero;
	camera->fov = 70.0f;
	camera->speed = 50.0f;
	camera->sensitivity = 0.1f;
	camera->pos_lerp = 1.0f;
	camera->rot_lerp = 1.0f;

	return 1;
}

void graphics_free(graphics_t* graphics)
{
	glDeleteVertexArrays(1, &graphics->vao);
	glDeleteBuffers(1, &graphics->vbo);
	glDeleteBuffers(1, &graphics->ebo);

	gltDeleteText(graphics->text);
	gltTerminate();
}

void graphics_render(graphics_t* graphics, game_t* game)
{
	camera_t* camera = &graphics->camera;

	mat44_init_view(&camera->view, &camera->front, &camera->right, &camera->up, &camera->pos);
	/* TODO get actual values from the game */
	mat44_init_proj(&camera->proj, camera->fov, graphics->aspect, 16.0f, 100000.0f);
	mat44_mulm(&camera->view, &camera->proj, &camera->viewproj);

	/* TODO glEnable(GL_CULL_FACE) */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.25f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (game->player_count > 0)
		graphics_draw_vehicle(graphics, game->players[0]->vehicle);
	if (game->course.kcl.tri_count > 0)
		graphics_draw_kcl(graphics, &game->course.kcl);
	graphics_draw_overlay(graphics, game);

	for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
		printf("OpenGL Error (%d): %s\n", error, glewGetErrorString(error));
}

void graphics_draw_sphere(graphics_t* graphics, int rings, int sectors, const vec3_t* pos, const vec3_t* color, float radius)
{
	double dR = (double)rings;
	double dS = (double)sectors;
	double R = 1.0f / (dR - 1.0);
	double S = 1.0f / (dS - 1.0);

	typedef struct
	{
		vec3_t pos;
		vec3_t norm;
	} sphere_vertex_t;

	const int vertex_count = rings * sectors;
	sphere_vertex_t* vertices = malloc(vertex_count * sizeof(sphere_vertex_t));
	sphere_vertex_t* v = vertices;

	for (double r = 0; r < dR; ++r)
	{
		for (double s = 0; s < dS; ++s)
		{
			v->norm.x = (float)(cos(M_PI * 2.0 * s * S) * sin(M_PI * r * R));
			v->norm.y = (float)(sin(-(M_PI / 2.0) + M_PI * r * R));
			v->norm.z = (float)(sin(M_PI * 2.0 * s * S) * sin(M_PI * r * R));
			vec3_mul(&v->norm, radius, &v->pos);
			v++;
		}
	}

	const int index_count = rings * sectors * 6;
	unsigned int* indices = malloc(index_count * sizeof(unsigned int));
	unsigned int* i = indices;
	for (int r = 0; r < rings - 1; ++r)
	{
		for (int s = 0; s < sectors - 1; ++s)
		{
			*i++ = r * sectors + s;
			*i++ = r * sectors + (s + 1);
			*i++ = (r + 1) * sectors + (s + 1);
			*i++ = (r + 1) * sectors + (s + 1);
			*i++ = (r + 1) * sectors + s;
			*i++ = r * sectors + s;
		}
	}

	shader_basic_t* shader = graphics->shader_basic;
	glUseProgram(shader->base.program);

	mat44_t model, mv, mvp;
	mat44_init_pos(&model, pos);
	mat44_mulm(&model, &graphics->camera.view, &mv);
	mat44_mulm(&mv, &graphics->camera.proj, &mvp);

	glUniformMatrix4fv(shader->uniform_mvp, 1, GL_FALSE, mvp.m);
	glUniform3fv(shader->uniform_color, 1, color->v);
	glUniform3fv(shader->uniform_light_dir, 1, graphics->sun_dir.v);

	glBindVertexArray(graphics->vao);
	glBindBuffer(GL_ARRAY_BUFFER, graphics->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(*vertices) * vertex_count, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices) * index_count, indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader->attribute_pos);
	glVertexAttribPointer(shader->attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(*vertices), (void*)offsetof(sphere_vertex_t, pos));

	glEnableVertexAttribArray(shader->attribute_norm);
	glVertexAttribPointer(shader->attribute_norm, 3, GL_FLOAT, GL_FALSE, sizeof(*vertices), (void*)offsetof(sphere_vertex_t, norm));

	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glDisableVertexAttribArray(shader->attribute_pos);
	glDisableVertexAttribArray(shader->attribute_norm);

	free(vertices);
	free(indices);
}

void graphics_draw_vehicle(graphics_t* graphics, vehicle_t* vehicle)
{
	vec3_t wheel_color = { 1.0f, 0.0f, 0.0f };
	for (int i = 0; i < vehicle->wheel_count; i++)
	{
		vehicle_wheel_t* wheel = &vehicle->wheels[i];
		graphics_draw_sphere(graphics, 16, 16, &wheel->pos, &wheel_color, wheel->bsp.sphere_radius);
	}

	vec3_t hitbox_color = { 1.0f, 1.0f, 0.5f };
	for (int i = 0; i < vehicle->body.hitbox_count; i++)
	{
		hitbox_t* hitbox = &vehicle->body.hitboxes[i];
		graphics_draw_sphere(graphics, 16, 16, &hitbox->pos, &hitbox_color, hitbox->radius);
	}
}

void graphics_draw_kcl(graphics_t* graphics, kcl_t* kcl)
{
	shader_basic_vcolor_t* shader = graphics->shader_basic_vcolor;
	glUseProgram(shader->base.program);

	glUniformMatrix4fv(shader->uniform_mvp, 1, GL_FALSE, graphics->camera.viewproj.m);
	glUniform3fv(shader->uniform_light_dir, 1, graphics->sun_dir.v);

	glBindVertexArray(graphics->vao);
	glBindBuffer(GL_ARRAY_BUFFER, graphics->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(*kcl->vertices) * 3 * kcl->vertex_count, kcl->vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader->attribute_pos);
	glVertexAttribPointer(
		shader->attribute_pos,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(kcl_tri_vertex_t),
		(void*)offsetof(kcl_tri_vertex_t, position)
	);

	glEnableVertexAttribArray(shader->attribute_norm);
	glVertexAttribPointer(
		shader->attribute_norm,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(kcl_tri_vertex_t),
		(void*)offsetof(kcl_tri_vertex_t, normal)
	);

	glEnableVertexAttribArray(shader->attribute_color);
	glVertexAttribPointer(
		shader->attribute_color,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(kcl_tri_vertex_t),
		(void*)offsetof(kcl_tri_vertex_t, color)
	);

	glDrawArrays(GL_TRIANGLES, 0, kcl->vertex_count * 3);

	glBindVertexArray(0);
	glDisableVertexAttribArray(shader->attribute_pos);
	glDisableVertexAttribArray(shader->attribute_norm);
	glDisableVertexAttribArray(shader->attribute_color);
}

void graphics_draw_overlay(graphics_t* graphics, game_t* game)
{
	player_t* player = game->player_count > 0 ? game->players[0] : NULL;
	camera_t* camera = &graphics->camera;
	GLTtext* text = graphics->text;

	glDisable(GL_DEPTH_TEST);
	gltBeginDraw();
	gltColor(1.0f, 1.0f, 1.0f, 1.0f);

	float x = 0.0f, y = 0.0f, scale = 1.0f;
	float y_inc = gltGetLineHeight(scale);
	y -= y_inc;

#define gltDrawText2DFormatAdvance(...) \
	do { y += y_inc; gltDrawText2DFormat(__VA_ARGS__); } while (0)

	gltDrawText2DFormatAdvance(text, x, y, scale, "FPS    %.0f", 1.0 / (graphics->frametime / 1000.0));
	y += y_inc;

	gltDrawText2DFormatAdvance(text, x, y, scale, "Pos    %.2f %.2f %.2f", XYZ(camera->pos));
	gltDrawText2DFormatAdvance(text, x, y, scale, "Quat   %.2f %.2f %.2f %.2f", XYZW(camera->quat));
	gltDrawText2DFormatAdvance(text, x, y, scale, "Ang    %.2f %.2f %.2f", XYZ(camera->angles));
	y += y_inc;

	if (game->ghost.frame_count != 0)
	{
		gltDrawText2DFormatAdvance(text, x, y, scale, "Ghost  %s", game->ghost.name);
		gltDrawText2DFormatAdvance(text, x, y, scale, "Course %s", course_name_by_id(game->ghost.header.course_id));
		gltDrawText2DFormatAdvance(text, x, y, scale, "Frame  %u/%u", game->frame_idx, game->keyframes.frame_count);

		if (game->keyframes.frame_desync != UINT32_MAX)
			gltDrawText2DFormatAdvance(text, x, y, scale, "Desync %u", game->keyframes.frame_desync);

		y += y_inc;
	}

	if (game->override_input)
		gltDrawText2DFormatAdvance(text, x, y, scale, "Freeroam");
	if (player && player->freecam)
		gltDrawText2DFormatAdvance(text, x, y, scale, "Freecam");
	if (game->pause)
		gltDrawText2DFormatAdvance(text, x, y, scale, "PAUSED");
	y += y_inc;

	if (player)
	{
		physics_t* physics = &player->vehicle->physics;
		gltDrawText2DFormatAdvance(text, x, y, scale, "pos       %.1f %.1f %.1f\n", XYZ(physics->pos));
		gltDrawText2DFormatAdvance(text, x, y, scale, "dir       %.1f %.1f %.1f\n", XYZ(physics->dir));
		gltDrawText2DFormatAdvance(text, x, y, scale, "vel       %.1f %.1f %.1f\n", XYZ(physics->vel));
		gltDrawText2DFormatAdvance(text, x, y, scale, "vel0      %.1f %.1f %.1f\n", XYZ(physics->vel0));
		gltDrawText2DFormatAdvance(text, x, y, scale, "vel1      %.1f %.1f %.1f\n", XYZ(physics->vel1));
		gltDrawText2DFormatAdvance(text, x, y, scale, "vel1 dir  %.1f %.1f %.1f\n", XYZ(physics->vel1_dir));
		gltDrawText2DFormatAdvance(text, x, y, scale, "main rot  %.1f %.1f %.1f %.1f\n", XYZW(physics->main_rot));
		gltDrawText2DFormatAdvance(text, x, y, scale, "full rot  %.1f %.1f %.1f %.1f\n", XYZW(physics->full_rot));
		gltDrawText2DFormatAdvance(text, x, y, scale, "rot vec0  %.1f %.1f %.1f\n", XYZ(physics->rot_vec0));
		gltDrawText2DFormatAdvance(text, x, y, scale, "rot vec2  %.1f %.1f %.1f\n", XYZ(physics->rot_vec2));
		gltDrawText2DFormatAdvance(text, x, y, scale, "s up      %.1f %.1f %.1f\n", XYZ(physics->smoothed_up));
		gltDrawText2DFormatAdvance(text, x, y, scale, "accel     %.1f\n", physics->normal_acceleration);
		gltDrawText2DFormatAdvance(text, x, y, scale, "speed1    %.1f\n", physics->speed1);
		gltDrawText2DFormatAdvance(text, x, y, scale, "speed1 a  %.1f\n", physics->speed1_adj);
		gltDrawText2DFormatAdvance(text, x, y, scale, "speed1 l  %.1f\n", physics->last_speed1);
	}

#undef gltDrawText2DFormatAdvance
	gltEndDraw();
}
