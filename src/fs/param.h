#pragma once

enum
{
    driver_weight_light,
    driver_weight_medium,
    driver_weight_heavy,
};

typedef struct param_section_t
{
    int32_t num_tires;
    int32_t drift_type;
    int32_t weight_class;
    float   unknown;
    float   weight;
    float   bump_deviation;
    float   speed;
    float   speed_in_turn;
    float   tilt;
    float   acceleration_ys[4];
    float   acceleration_xs[4];
    float   drift_acceleration_ys[2];
    float   drift_acceleration_xs[2];
    float   handling_tightness_manual;
    float   handling_tightness_auto;
    float   handling_reactivity;
    float   drift_tightness_manual;
    float   drift_tightness_auto;
    float   drift_reactivity;
    float   drift_target_angle;
    float   drift_decrement;
    int32_t mini_turbo_duration;
    float   speed_multipliers[32];
    float   rotation_multipliers[32];
    float   rotating_items_z_radius;
    float   rotating_items_x_radius;
    float   rotating_trailing_items_y_dist;
    float   rotating_trailing_items_z_dist;
    float   max_normal_acceleration;
    float   mega_mushroom_scale;
    float   rear_to_front_tire_dist;
} param_section_t;

void param_merge(param_section_t* param, param_section_t* other, param_section_t* out);

typedef struct param_t
{
    param_section_t*    sections;
	uint32_t		    section_count;
} param_t;

extern parser_t param_parser;