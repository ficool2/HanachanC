#include "../common.h"
#include "param.h"

void param_merge(param_section_t* param, param_section_t* other, param_section_t* out)
{
    *out = *param;

    out->weight        += other->weight;
    out->speed         += other->speed;
    out->speed_in_turn += other->speed_in_turn;

    for (int i = 0; i < 4; i++)
        out->acceleration_ys[i]       += other->acceleration_ys[i];
    for (int i = 0; i < 3; i++)
        out->acceleration_xs[i]       += other->acceleration_xs[i];
    for (int i = 0; i < 2; i++)
        out->drift_acceleration_ys[i] += other->drift_acceleration_ys[i];
    for (int i = 0; i < 1; i++)
        out->drift_acceleration_xs[i] += other->drift_acceleration_xs[i];

    out->handling_tightness_manual += other->handling_tightness_manual;
    out->handling_tightness_auto   += other->handling_tightness_auto;
    out->handling_reactivity       += other->handling_reactivity;
    out->drift_tightness_manual    += other->drift_tightness_manual;
    out->drift_tightness_auto      += other->drift_tightness_auto;
    out->drift_reactivity          += other->drift_reactivity;
    out->mini_turbo_duration       += other->mini_turbo_duration;

    for (int i = 0; i < 32; i++)
    {
        out->speed_multipliers[i]    += other->speed_multipliers[i];
        out->rotation_multipliers[i] += other->rotation_multipliers[i];
    }
}

void param_init(param_t* param)
{
	param->sections = NULL;
	param->section_count = 0;
}

void param_free(param_t* param)
{
	free(param->sections);
	param->sections = NULL;
	param->section_count = 0;
}

int param_parse(param_t* param, bin_t* bin)
{
	bswapstream_t stream;
	bswapstream_init(&stream, bin->buffer);

	param->section_count = bswapstream_read_uint32(&stream);
	param->sections		 = malloc(param->section_count * sizeof(*param->sections));

	for (uint32_t i = 0; i < param->section_count; i++)
	{
		param_section_t* section                = &param->sections[i];

        section->num_tires                      = bswapstream_read_int32(&stream);
        section->drift_type                     = bswapstream_read_int32(&stream);
        section->weight_class                   = bswapstream_read_int32(&stream);
        section->unknown                        = bswapstream_read_float(&stream);
        section->weight                         = bswapstream_read_float(&stream);
        section->bump_deviation                 = bswapstream_read_float(&stream);
        section->speed                          = bswapstream_read_float(&stream);
        section->speed_in_turn                  = bswapstream_read_float(&stream);
        section->tilt                           = bswapstream_read_float(&stream);

        for (int j = 0; j < 4; j++)
            section->acceleration_ys[j]         = bswapstream_read_float(&stream);
        section->acceleration_xs[0]             = 0.0f;
        for (int j = 1; j < 4; j++)
            section->acceleration_xs[j]         = bswapstream_read_float(&stream);
        for (int j = 0; j < 2; j++)
            section->drift_acceleration_ys[j]   = bswapstream_read_float(&stream);
        section->drift_acceleration_xs[0] = 0.0f;
        for (int j = 1; j < 2; j++)
            section->drift_acceleration_xs[j]   = bswapstream_read_float(&stream);

        section->handling_tightness_manual      = bswapstream_read_float(&stream);
        section->handling_tightness_auto        = bswapstream_read_float(&stream);
        section->handling_reactivity            = bswapstream_read_float(&stream);
        section->drift_tightness_manual         = bswapstream_read_float(&stream);
        section->drift_tightness_auto           = bswapstream_read_float(&stream);
        section->drift_reactivity               = bswapstream_read_float(&stream);
        section->drift_target_angle             = bswapstream_read_float(&stream);
        section->drift_decrement                = bswapstream_read_float(&stream);
        section->mini_turbo_duration            = bswapstream_read_int32(&stream);

        for (int j = 0; j < 32; j++)
            section->speed_multipliers[j]       = bswapstream_read_float(&stream);
        for (int j = 0; j < 32; j++)
            section->rotation_multipliers[j]    = bswapstream_read_float(&stream);

        section->rotating_items_z_radius        = bswapstream_read_float(&stream);
        section->rotating_items_x_radius        = bswapstream_read_float(&stream);
        section->rotating_trailing_items_y_dist = bswapstream_read_float(&stream);
        section->rotating_trailing_items_z_dist = bswapstream_read_float(&stream);
        section->max_normal_acceleration        = bswapstream_read_float(&stream);
        section->mega_mushroom_scale            = bswapstream_read_float(&stream);
        section->rear_to_front_tire_dist        = bswapstream_read_float(&stream);
	}

    return 1;
}

parser_t param_parser =
{
    param_init,
    param_free,
    param_parse
};