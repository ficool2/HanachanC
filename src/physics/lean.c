#include "../common.h"
#include "../vehicle/vehicle.h"
#include "../game/game.h"

#include "lean.h"

lean_params_t params_outside_drift =
{
    .rot_params[0] = { .inc = 0.08f, .cap = 1.0f, },
    .rot_params[1] = { .inc = 0.08f, .cap = 0.6f, },
    .rot_params[2] = { .inc = 0.15f, .cap = 1.6f, },
    .rot_min        = 0.8f,
    .rot_max        = 1.2f,
    .stick_x_factor = 0.1f,
    .s_factor       = 0.8f,
};

lean_params_t params_inside_drift =
{
    .rot_params[0] = { .inc = 0.1f,  .cap = 1.0f, },
    .rot_params[1] = { .inc = 0.08f, .cap = 0.6f, },
    .rot_params[2] = { .inc = 0.15f, .cap = 1.3f, },
   .rot_min        = 0.7f,
   .rot_max        = 1.5f,
   .stick_x_factor = 0.05f,
   .s_factor       = 1.0f,
};

void lean_init(lean_t* lean)
{
    lean->rot      = 0.0f;
    lean->rot_diff = 0.08f;
    lean->rot_cap  = 0.6f;
    lean->params   = NULL;
}

void lean_set_drift(lean_t* lean, bool inside_drift)
{
    lean->params   = inside_drift ? &params_inside_drift : &params_outside_drift;
}

void lean_update(lean_t* lean, vehicle_t* vehicle, float stick_x, int stage)
{   
    lean_params_t* params = lean->params;
    physics_t* physics = &vehicle->physics;

    lean_rot_params_t* rot_params;
    if (vehicle->standstill_miniturbo.charging)
        rot_params = &params->rot_params[2];
    else if (stage == stage_race && fabsf(physics->speed1) >= 5.0f)
        rot_params = &params->rot_params[0];
    else
        rot_params = &params->rot_params[1];

    lean->rot_diff += 0.3f * (rot_params->inc - lean->rot_diff);
    lean->rot_cap  += 0.3f * (rot_params->cap - lean->rot_cap);
    
    uint32_t airtime = vehicle->floor.airtime;
    float rot_min, rot_max, s;
    if (vehicle->drift.state == drift_normal && airtime <= 20)
    {
        float drift_stick_x = vehicle->drift.drift.stick_x;
        if (stick_x == 0.0f)
            lean->rot += 0.05f * (0.5f * drift_stick_x - lean->rot);
        else
            lean->rot += params->stick_x_factor * stick_x;

        if (drift_stick_x < 0.0f)
        {
            rot_min  = -params->rot_max;
            rot_max  = -params->rot_min;
        }
        else
        {
            rot_min  = params->rot_min;
            rot_max  = params->rot_max;
        }

        s = params->s_factor * -stick_x;
    }
    else
    {
        rot_min = -lean->rot_cap;
        rot_max = lean->rot_cap;

        if (fabsf(stick_x) <= 0.2f || airtime > 20 || vehicle->wheelie.is_wheelieing)
        {
            lean->rot  *= 0.9f;
            s           = 0.0f;
        }
        else 
        {
            float sign  = -copysignf(1.0f, stick_x);
            lean->rot  -= sign * lean->rot_diff;
            s           = params->s_factor * sign;
        }
    }

    if (lean->rot < rot_min)
    {
        lean->rot = rot_min;
    }
    else if (lean->rot > rot_max)
    {
        lean->rot = rot_max;
    }
    else 
    {
        mat33_t m;
        vec3_t right;
        mat33_init_mat34(&m, &physics->mat);
        mat33_mulv(&m, &vec3_right, &right);
        vec3_mul(&right, s, &right);
        vec3_add(&physics->vel0, &right, &physics->vel0);
    }

    float drift_factor = vehicle->drift.state == drift_normal ? 1.3f : 1.0f;
    physics->rot_vec2.z += 0.05f * drift_factor * lean->rot;
}