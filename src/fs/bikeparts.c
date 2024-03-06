#include "../common.h"
#include "bikeparts.h"

void bikeparts_init(bikeparts_t* parts)
{
	parts->sections = NULL;
	parts->section_count = 0;
}

void bikeparts_free(bikeparts_t* parts)
{
	free(parts->sections);
	parts->sections = NULL;
	parts->section_count = 0;
}

int bikeparts_parse(bikeparts_t* parts, bin_t* bin)
{
    bswapstream_t stream;
    bswapstream_init(&stream, bin->buffer);

    parts->section_count = bswapstream_read_uint32(&stream);
    parts->sections = malloc(parts->section_count * sizeof(*parts->sections));

    for (uint32_t i = 0; i < parts->section_count; i++)
    {
        bikeparts_section_t* section           = &parts->sections[i];
        bikepart_handle_t* handle              = &section->handle;

        section->camera_target_vertical_dist   = bswapstream_read_float(&stream);
        section->tires_forward_rotation_speed  = bswapstream_read_float(&stream);
        section->tires_backward_rotation_speed = bswapstream_read_float(&stream);
        handle->pos                            = bswapstream_read_vec3(&stream);
        handle->angles                         = bswapstream_read_vec3(&stream);
        for (int j = 0; j < 3; j++)
            handle->angles.v[j]                = radiansf(handle->angles.v[j]);
        for (int j = 0; j < 13; j++)
            section->unknown[j]                = bswapstream_read_float(&stream);
        section->exhaust1_pipes_count          = bswapstream_read_int32(&stream);
        section->exhaust1_pos                  = bswapstream_read_vec3(&stream);
        section->exhaust1_rot                  = bswapstream_read_vec2(&stream);
        section->exhaust2_pipes_count          = bswapstream_read_int32(&stream);
        section->exhaust2_pos                  = bswapstream_read_vec3(&stream);
        section->exhaust2_rot                  = bswapstream_read_vec2(&stream);
        section->ice_size                      = bswapstream_read_vec3(&stream);
        section->ice_pos                       = bswapstream_read_vec3(&stream);
        section->balloons_pos                  = bswapstream_read_vec2(&stream);
        section->unknown2                      = bswapstream_read_vec2(&stream);
    }

    return 1;
}

parser_t bikeparts_parser =
{
	bikeparts_init,
	bikeparts_free,
	bikeparts_parse,
};