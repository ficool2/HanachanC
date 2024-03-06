#include "../common.h"
#include "../physics/physics.h"
#include "rkrd.h"

void rkrd_init(rkrd_t* rkrd)
{
    rkrd->frames = NULL;
    rkrd->frame_count = 0;
    rkrd->frame_desync = UINT32_MAX;
}

void rkrd_free(rkrd_t* rkrd)
{
    free(rkrd->frames);
    rkrd->frames = NULL;
    rkrd->frame_count = 0;
    rkrd->frame_desync = UINT32_MAX;
}

int rkrd_parse(rkrd_t* rkrd, bin_t* rkrd_buffer)
{
    rkrd_header_t* header = (rkrd_header_t*)rkrd_buffer->buffer;
    if (strncmp(header->id, "RKRD", 4))
    {
        printf("Error reading RKRD, bad header\n");
        return 0;
    }

    bswapstream_t stream;
    bswapstream_init(&stream, rkrd_buffer->buffer);
    bswapstream_read_uint32(&stream);

    uint32_t version = bswapstream_read_uint32(&stream);
    if (version != 2)
    {
        printf("Error reading RKRD, bad version\n");
        return 0;
    }

    uint32_t size = (uint32_t)rkrd_buffer->size - sizeof(rkrd_header_t);
    if (size % sizeof(rkrd_frame_t))
    {
        printf("Error reading RKRD, bad frame total size\n");
        return 0;
    }

    rkrd->frame_count = size / sizeof(rkrd_frame_t);
    rkrd->frames = malloc(rkrd->frame_count * sizeof(rkrd_frame_t));

    for (size_t i = 0; i < rkrd->frame_count; i++)
    {
        rkrd_frame_t* frame         = &rkrd->frames[i];
        frame->rot_vec2             = bswapstream_read_vec3(&stream);
        frame->speed1_soft_limit    = bswapstream_read_float(&stream);
        frame->speed1               = bswapstream_read_float(&stream);
        frame->floor_nor            = bswapstream_read_vec3(&stream);
        frame->dir                  = bswapstream_read_vec3(&stream);
        frame->pos                  = bswapstream_read_vec3(&stream);
        frame->vel0                 = bswapstream_read_vec3(&stream);
        frame->rot_vec0             = bswapstream_read_vec3(&stream);
        frame->vel2                 = bswapstream_read_vec3(&stream);
        frame->vel                  = bswapstream_read_vec3(&stream);
        frame->main_rot             = bswapstream_read_quat(&stream);
        frame->full_rot             = bswapstream_read_quat(&stream);
        frame->animation            = bswapstream_read_uint16(&stream);
        frame->checkpoint_idx       = bswapstream_read_uint16(&stream);
    }

    int ret = 1;
    uint32_t size_read = (uint32_t)bswapstream_current_read(&stream) - sizeof(rkrd_header_t);
    if (size_read != size)
    {
        printf("Error reading input data (expected: %u, got %u)\n", size, size_read);
        ret = 0;
    }

    return 1;
}

bool rkrd_check_desync(rkrd_frame_t* frame, physics_t* physics)
{
	bool ret = true;

	#define check_float(actual, expected) \
	if (actual != expected) \
	{ \
		printf("Desync: " #actual " %.12f, expected %.12f\n", \
			actual, expected); \
		ret = false;\
	}
	#define check_vec3(actual, expected) \
	if (actual.x != expected.x || actual.y != expected.y || actual.z != expected.z) \
	{ \
		printf("Desync: " #actual " %.12f %.12f %.12f, expected %.12f %.12f %.12f\n", \
			actual.x, actual.y, actual.z, \
			expected.x, expected.y, expected.z); \
		ret = false;\
	}
	#define check_quat(actual, expected) \
	if (actual.x != expected.x || actual.y != expected.y || actual.z != expected.z || actual.w != expected.w) \
	{ \
		printf("Desync: " #actual " %.12f %.12f %.12f %.12f, expected %.12f %.12f %.12f %.12f\n", \
			actual.x, actual.y, actual.z, actual.w, \
			expected.x, expected.y, expected.z, expected.w); \
		ret = false;\
	}

	check_vec3(physics->up,       frame->floor_nor);
	check_vec3(physics->dir,      frame->dir);
	check_vec3(physics->pos,      frame->pos);
	check_vec3(physics->vel0,     frame->vel0);
	check_float(physics->speed1,  frame->speed1);
	check_vec3(physics->vel,      frame->vel);
	check_vec3(physics->rot_vec0, frame->rot_vec0);
	check_vec3(physics->rot_vec2, frame->rot_vec2);
	check_quat(physics->main_rot, frame->main_rot);
	check_quat(physics->full_rot, frame->full_rot);

	#undef check_float
	#undef check_vec3
	#undef check_quat

	return ret;
}

parser_t rkrd_parser =
{
    rkrd_init,
    rkrd_free,
    rkrd_parse,
};