#include "../common.h"
#include "physics.h"
#include "../vehicle/vehicle.h"
#include "../fs/bsp.h"
#include "../game/game.h"

void physics_init(physics_t* physics)
{
    memset(physics, 0, sizeof(*physics));
}
    
void physics_place(physics_t* physics, bsp_t* bsp, kmp_t* kmp, kcl_t* kcl)
{
    vec3_t inertia_tensor = vec3_zero, inertia_tensor_diag;
    float masses[] = { 1.0f / 12.0f, 1.0f };

    for (int i = 0; i < 2; i++)
    {
        vec3_t* cuboid = &bsp->cuboids[i];
        float mass = masses[i];

        vec3_t sum =
        {
            mass * (cuboid->y * cuboid->y + cuboid->z * cuboid->z),
            mass * (cuboid->z * cuboid->z + cuboid->x * cuboid->x),
            mass * (cuboid->x * cuboid->x + cuboid->y * cuboid->y)
        };

        vec3_add(&inertia_tensor, &sum, &inertia_tensor);
    }

    float det = inertia_tensor.x * inertia_tensor.y * inertia_tensor.z;
    float recip = 1.0f / det;
    inertia_tensor_diag = (vec3_t)
    {
        recip * (inertia_tensor.y * inertia_tensor.z),
        recip * (inertia_tensor.z * inertia_tensor.x),
        recip * (inertia_tensor.x * inertia_tensor.y),
    };

    mat34_init_diag(&physics->inv_inertia_tensor, &inertia_tensor_diag);

    ktpt_t* ktpt = &kmp->ktpt[0];

    vec3_t pos;
    vec3_t diff0 = { -800.0f, 0.0f,  461.87988f };
    vec3_t diff1 = { 800.0f,  0.0f, -461.87991f };
    vec3_add(&ktpt->position, &diff0, &pos);
    vec3_add(&pos, &diff1, &pos);

    hitbox_t hitbox;
    hitbox_init(&hitbox, &pos, NULL, 100.0f, 0x20E80FFF);

    collision_t collision;
    kcl_collision_hitbox(kcl, &hitbox, &collision);
    
    vec3_t movement, floor_offset;
    vec3_mul(&collision.floor_normal, 100.0f, &floor_offset);
    collision_movement(&collision, &movement);
    vec3_add(&pos, &movement, &movement);
    vec3_sub(&movement, &floor_offset, &pos);
    vec3_mul(&collision.floor_normal, bsp->y_offset, &floor_offset);
    vec3_add(&pos, &floor_offset, &pos);

    physics->rot_factor                 = bsp->angular_velocity_boost;
    mat34_init_quat_pos                 (&physics->mat, &quat_back, &pos);
    physics->up                         = vec3_up;
    physics->smoothed_up                = vec3_up;
    physics->dir                        = vec3_back;
    physics->dir_diff                   = vec3_zero;
    physics->vel1_dir                   = vec3_back;
    physics->landing_dir_valid          = false;
    physics->landing_dir                = vec3_zero;
    physics->landing_angle              = 0.0f;
    physics->pos                        = pos;
    physics->gravity                    = -1.3f;
    physics->normal_acceleration        = 0.0f;
    physics->vel0                       = vec3_zero;
    physics->vel1                       = vec3_zero;
    physics->last_speed1                = 0.0f;
    physics->speed1                     = 0.0f;
    physics->speed1_adj                 = 0.0f;
    physics->speed1_soft_limit          = 0.0f;
    physics->vel                        = vec3_zero;
    physics->normal_rot_vec             = vec3_zero;
    physics->rot_vec0                   = vec3_zero;
    physics->rot_vec2                   = vec3_zero;
    physics->main_rot                   = quat_back;
    physics->non_conserved_special_rot  = quat_identity;
    physics->conserved_special_rot      = quat_identity;
    physics->full_rot                   = quat_back;
    physics->stabilization_factor       = 0.0f;
}

void physics_update(physics_t* physics, vehicle_t* vehicle, int stage)
{
    if (stage != stage_race)
    {
        if (vehicle_is_bike(vehicle))
        {
            vec3_t vel0 = physics->vel0;
            vec3_rej_unit(&vel0, &physics->smoothed_up, &physics->vel0);
        }
        else
        {
            physics->vel0.x = 0.0f;
            physics->vel0.z = 0.0f;
        }
    }

    physics->vel0.y += physics->normal_acceleration + physics->gravity;
    physics->normal_acceleration = 0.0f;
    vec3_mul(&physics->vel0, 0.998f, &physics->vel0);
    vec3_mul(&physics->rot_vec0, 0.98f, &physics->rot_vec0);

    vec3_t front, front_xz;
    quat_rotate(&physics->main_rot, &vec3_front, &front);
    front_xz = (vec3_t){ front.x, 0.0f, front.z };

    if (vec3_magsqr(&front_xz) > FLT_EPSILON)
    {
        vec3_t proj, vel0;
        vel0 = physics->vel0;
        vec3_norm(&front_xz);
        vec3_proj_unit(&vel0, &front_xz, &proj);
        vec3_rej_unit(&vel0, &front_xz, &physics->vel0);

        physics->speed1_adj =
            copysignf(1.0f, vec3_dot(&front_xz, &proj))
            * vec3_mag(&proj)
            * vec3_dot(&front, &front_xz);
    }

    vec3_add(&physics->vel0, &physics->vel1, &physics->vel);
    float vel_mag = vec3_norm(&physics->vel);
    float speed = fminf(vel_mag, 120.0f);
    vec3_mul(&physics->vel, speed, &physics->vel);
    vec3_add(&physics->pos, &physics->vel, &physics->pos);

    mat33_t mat;
    vec3_t tmprot, tmp0, tmp1;
    vec3_t normal_rot_vec;
    mat33_init_mat34(&mat, &physics->inv_inertia_tensor);
    mat33_mulv(&mat, &physics->normal_rot_vec, &tmp0);
    vec3_add(&physics->normal_rot_vec, &tmp0, &tmprot);
    mat33_mulv(&mat, &tmprot, &tmp1);

    vec3_add(&tmp0, &tmp1, &normal_rot_vec);
    vec3_mul(&normal_rot_vec, 0.5f, &normal_rot_vec);
    vec3_add(&physics->rot_vec0, &normal_rot_vec, &physics->rot_vec0);
    physics->normal_rot_vec = vec3_zero;

    if (vehicle_is_bike(vehicle))
        physics->rot_vec0.z = 0.0;
    physics->rot_vec0.x = clampf(physics->rot_vec0.x, -0.4f, 0.4f);
    physics->rot_vec0.y = clampf(physics->rot_vec0.y, -0.4f, 0.4f);
    physics->rot_vec0.z = clampf(physics->rot_vec0.z, -0.8f, 0.8f);

    vec3_t rot_vec;
    vec3_mul(&physics->rot_vec0, physics->rot_factor, &rot_vec);
    vec3_add(&rot_vec, &physics->rot_vec2, &rot_vec);

    if (vec3_magsqr(&rot_vec) > FLT_EPSILON)
    {
        quat_t q = { rot_vec.x, rot_vec.y, rot_vec.z, 0.0f };
        quat_t p;
        quat_mulq(&physics->main_rot, &q, &p);
        quat_mulf(&p, 0.5f, &p);
        quat_addq(&physics->main_rot, &p, &physics->main_rot);

        if (quat_magsqr(&physics->main_rot) >= FLT_EPSILON)
            quat_norm(&physics->main_rot);
        else
            physics->main_rot = quat_identity;
    }

    physics_stabilize(physics, vehicle);

    /* this seems unnecessary if no stabilization was done */
    if (quat_magsqr(&physics->main_rot) > FLT_EPSILON)
        quat_norm(&physics->main_rot);
    else
        physics->main_rot = quat_identity;
    
    quat_t special_rot, conserved_special_rot;
    conserved_special_rot = physics->conserved_special_rot;

    quat_mulq(&physics->non_conserved_special_rot, &physics->conserved_special_rot, &special_rot);
    quat_mulq(&physics->main_rot, &special_rot, &physics->full_rot);
    quat_norm(&physics->full_rot);
    physics->non_conserved_special_rot = quat_identity;
    quat_slerp(&conserved_special_rot, &quat_identity, 0.1f, &physics->conserved_special_rot);
}

void physics_update_ups(physics_t* physics, vehicle_t* vehicle)
{
    floor_t* floor = &vehicle->floor;

    physics->landing_dir_valid = false;
    physics->stabilization_factor = 0.1f;

    if (floor_is_landing(floor) && floor->last_airtime >= 3)
    {
        physics->up = floor->normal;
        physics->smoothed_up = physics->up;

        vec3_cross_plane(&physics->dir, &physics->smoothed_up, &physics->landing_dir);
        vec3_norm(&physics->landing_dir);
        vec3_proj_unit(&physics->landing_dir, &physics->landing_dir, &physics->dir_diff);
        physics->landing_dir_valid = true;
    } 
    else if (vehicle->drift.state == drift_hop && vehicle->drift.hop.pos_y > 0.0f)
    {
        physics->stabilization_factor = vehicle->drift.has_outside_drift ? 0.5f : 0.22f;
    } 
    else if (floor->airtime > 20)
    {
        vec3_t delta;

        if (physics->up.y > 0.99f)
        {
            physics->up = vec3_up;
        } 
        else 
        {
            vec3_sub(&vec3_up, &physics->up, &delta);
            vec3_mul(&delta, 0.03f, &delta);
            vec3_add(&physics->up, &delta, &physics->up);
            vec3_norm(&physics->up);
        }
    
        if (physics->smoothed_up.y > 0.99f)
        {
            physics->smoothed_up = vec3_up;
        } 
        else 
        {
            vec3_sub(&vec3_up, &physics->smoothed_up, &delta);
            vec3_mul(&delta, 0.03f, &delta);
            vec3_add(&physics->smoothed_up, &delta, &physics->smoothed_up);
            vec3_norm(&physics->smoothed_up);
        }
    } 
    else if (floor->airtime == 0)
    {
        vec3_t front;
        mat33_t m;

        vec3_t* up;
        if ((vehicle->surface_props.has_boost_ramp || floor->trickable_timer > 0)
            && physics->speed1 > 50.0f
            && vec3_dot(&floor->normal, &physics->dir) > 0.0f
            && vehicle->surface_props.has_non_trickable)
        {
            up = &physics->up;
        }
        else
        {
            physics->up = floor->normal;
            up = &floor->normal;
        }
    
        float smoothing_factor;
        if (boost_type(&vehicle->boost) != boost_none || vehicle->wheelie.is_wheelieing)
        {
            smoothing_factor = 0.8f;
        }
        else
        {
            mat33_init_mat34(&m, &physics->mat);
            mat33_mulv(&m, &vec3_front, &front);

            smoothing_factor = 0.8f - 6.0f * fabsf(vec3_dot(up, &front));
            smoothing_factor = clampf(smoothing_factor, 0.3f, 0.8f);
        }

        vec3_t up_delta;
        vec3_sub(up, &physics->smoothed_up, &up_delta);
        vec3_mul(&up_delta, smoothing_factor, &up_delta);
        vec3_add(&physics->smoothed_up, &up_delta, &physics->smoothed_up);
        vec3_norm(&physics->smoothed_up);
    
        mat33_init_mat34(&m, &physics->mat);
        mat33_mulv(&m, &vec3_front, &front);
        float dot = vec3_dot(&front, &physics->smoothed_up);
        if (dot < -0.1f)
        {
            float factor = fabsf(dot) * 0.5f;
            physics->stabilization_factor += fminf(factor, 0.2f);
        }
    
        if (vehicle->surface_props.has_boost_ramp)
            physics->stabilization_factor = 0.4f;
    }
}

void physics_update_dirs(physics_t* physics, vehicle_t* vehicle)
{
    physics->vel1_dir = physics->dir;

    floor_t* floor = &vehicle->floor;
    if (floor->airtime > 0 && vehicle->surface_props.has_boost_ramp)
        return;
    if (floor->airtime > 5 || vehicle->jump_pad.variant != variant_invalid)
        return;
    if (vehicle->trick.state == trick_state_started)
        return;

    vec3_t next_dir;
    if (vehicle->drift.state == drift_hop)
    {
        next_dir = vehicle->drift.hop.dir;
    }
    else
    {
        vec3_t right;
        quat_rotate(&physics->main_rot, &vec3_right, &right);
        vec3_cross(&right, &physics->smoothed_up, &next_dir);
        vec3_norm(&next_dir);
    }

    float angle = physics->landing_angle;
    if (vehicle->drift.has_outside_drift)
        angle += vehicle->drift.outside.angle;
    angle = radiansf(angle);

    mat34_t mat34;
    mat33_t mat;
    vec3_t next_dir_world, next_dir_perp, next_dir_delta;
    mat34_init_axis_angle(&mat34, &physics->smoothed_up, angle);
    mat33_init_mat34(&mat, &mat34);
    mat33_mulv(&mat, &next_dir, &next_dir_world);
    vec3_cross_plane(&next_dir_world, &physics->smoothed_up, &next_dir_perp);
    vec3_norm(&next_dir_perp);
    vec3_sub(&next_dir_perp, &physics->dir, &next_dir_delta);

    if (vec3_magsqr(&next_dir_delta) < FLT_EPSILON)
    {
        physics->dir = next_dir_perp;
        physics->dir_diff = vec3_zero;
    }
    else
    {
        vec3_t axis, next_axis;
        vec3_cross(&physics->dir, &next_dir_perp, &axis);
        vec3_mul(&next_dir_delta, floor->rotation_factor, &next_dir_delta);
        vec3_add(&next_dir_delta, &physics->dir_diff, &next_dir_delta);
        vec3_add(&physics->dir, &next_dir_delta, &physics->dir);
        vec3_norm(&physics->dir);
        vec3_mul(&next_dir_delta, 0.1f, &physics->dir_diff);

        vec3_cross(&physics->dir, &next_dir_perp, &next_axis);
        if (vec3_dot(&axis, &next_axis) < 0.0f)
        {
            physics->dir = next_dir_perp;
            physics->dir_diff = vec3_zero;
        }
    }

    vec3_cross_plane(&physics->dir, &physics->smoothed_up, &physics->vel1_dir);
    vec3_norm(&physics->vel1_dir);
}

void physics_update_accel(physics_t* physics, vehicle_t* vehicle, input_t* input, int stage)
{
    param_section_t* stats = &vehicle->stats;
    boost_t* boost = &vehicle->boost;
    floor_t* floor = &vehicle->floor;

    if (vehicle->drift.state != drift_normal && stage == stage_race)
        physics->speed1 += physics->speed1_adj;

    if (physics->speed1 < -20.0f)
        physics->speed1 += 0.5f;

    uint32_t airtime = floor->airtime;
    bool grounded = airtime == 0;
    float acceleration = 0.0f;
    if (!grounded || vehicle->standstill_miniturbo.charging)
    {
        if (vehicle->ramp_boost.duration > 0 && airtime < 4)
        {
            acceleration = 7.0f;
        }
        else if (vehicle->jump_pad.variant == variant_invalid || input->accelerate)
        {
            if (airtime > 5)
                physics->speed1 *= 0.999f;
        }
        else
        {
            physics->speed1 *= 0.99f;
        }
    }
    else
    {
        float boost_accel = boost_acceleration(boost);
        if (boost_accel != 0.0f)
        {
            acceleration = boost_accel;
        }
        else if (vehicle->ramp_boost.duration > 0
              || variant_jump_pad_speed(vehicle->jump_pad.variant) != 0.0f)
        {
            acceleration = 7.0f;
        }
        else
        {
            if (stage == stage_race && input->accelerate)
            {
                acceleration = physics_calc_acceleration(physics, vehicle);
            }
            else if (stage == stage_race && input->brake)
            {
                if (physics->driving_dir == driving_dir_braking)
                {
                    acceleration = -1.5f;
                }
                else if (physics->driving_dir == driving_dir_wait_backward
                      && ++physics->driving_backward_frame > 15)
                {
                    physics->driving_dir = driving_dir_backward;
                }
                else if (physics->driving_dir == driving_dir_backward)
                {
                    acceleration = -2.0f;
                }
            }
            else
            {   
                physics->speed1 *= physics->speed1 > 0.0 ? 0.98f : 0.95f;
            }

            if (vehicle->drift.state != drift_normal)
            {
                float t = stats->speed_in_turn;
                float raw_turn = fabsf(vehicle->turn.raw);
                physics->speed1 *= t + (1.0f - t) * (1.0f - raw_turn * physics->speed1_ratio);
            }
        }
    }

    physics->last_speed1 = physics->speed1;

    if (acceleration < 0.0f)
    {
        const float min_speed = -20.0f;
        if (physics->speed1 < min_speed)
            acceleration = 0.0f;  
        else if (physics->speed1 + acceleration <= min_speed)
            acceleration = fmaxf(acceleration, min_speed - physics->speed1);
    }

    physics->speed1 += acceleration;

    if (!vehicle->standstill_miniturbo.charging)
    {
        if (physics->driving_dir == driving_dir_braking && physics->speed1 < 0.0f)
        {
            physics->speed1 = 0.0f;
            physics->driving_dir = driving_dir_wait_backward;
            physics->driving_backward_frame = 0;
        }
    }
    else
    {
        physics->speed1 *= 0.8f;
    }

    float base_speed;
    float jump_pad_speed = variant_jump_pad_speed(vehicle->jump_pad.variant);
    if (jump_pad_speed != 0.0f)
        base_speed = jump_pad_speed;
    else
        base_speed = stats->speed;

    float boost_multi = boost_factor(boost);
    float wheelie_bonus = vehicle->wheelie.is_wheelieing ? 0.15f : 0.0f;
    float next_soft_limit = (boost_multi + wheelie_bonus) * floor->speed_factor * base_speed;
   
    float boost_lim = boost_limit(boost);
    if (boost_lim != 0.0f && jump_pad_speed == 0.0f)
        next_soft_limit = fmaxf(next_soft_limit, boost_lim * floor->speed_factor);

    if (vehicle->ramp_boost.duration > 0)
        next_soft_limit = fmaxf(next_soft_limit, 100.0f);

    physics->speed1_soft_limit = fmaxf(physics->speed1_soft_limit - 3.0f, next_soft_limit);
    physics->speed1_soft_limit = fminf(physics->speed1_soft_limit, 120.0f);
    physics->speed1 = fminf(physics->speed1, physics->speed1_soft_limit);

    if (jump_pad_speed != 0.0f)
        physics->speed1 = fmaxf(physics->speed1, jump_pad_speed);

    physics->speed1_ratio = fminf(fabsf(physics->speed1 / vehicle->stats.speed), 1.0f);

    vec3_t right;
    vec3_cross(&physics->smoothed_up, &physics->dir, &right);

    if (physics->speed1 < 0.0f)
        vec3_mul(&right, -1.0f, &right);
        
    float angle;
    if (vehicle->surface_props.has_boost_ramp)
        angle = 4.0f;
    else if (grounded)
        angle = 0.5f;
    else
        angle = 0.2f;
    angle = radiansf(angle);

    if (floor->airtime == 0 && !input->accelerate && stage >= stage_race)
    {
        if (physics->smoothed_up.y > 0.0f && fabsf(physics->speed1) < 30.0f)
        {
            float diff = 1.0f - physics->smoothed_up.y;
            if (diff > 0.0f)
            {
                vec3_t front;
                mat34_front(&physics->mat, &front);
                front.y *= -1.0f;

                float friction = -0.5f;
                if (front.y >= friction)
                    friction = fminf(front.y, 0.5f);

                physics->speed1 += fminf(diff * 2.0f, 2.0f) * friction;
            }
        }
    }

    mat34_t mat34;
    mat33_t mat33;
    vec3_t vel1_dir = physics->vel1_dir;
    mat34_init_axis_angle(&mat34, &right, angle);
    mat33_init_mat34(&mat33, &mat34);
    mat33_mulv(&mat33, &vel1_dir, &physics->vel1_dir);
    vec3_mul(&physics->vel1_dir, physics->speed1, &physics->vel1);

    if (floor->airtime > 0 || vehicle->drift.state == drift_normal || vehicle->drift.state == drift_hop)
    {
        physics->driving_dir = driving_dir_forward;
    }
    else if (!input->brake)
    {
        if (physics->speed1 >= 0.0f)
            physics->driving_dir = driving_dir_forward;
    }
    else if (physics->driving_dir == driving_dir_forward)
    {
        if (physics->speed1 > 5.0f)
            physics->driving_dir = driving_dir_braking;
        else
            physics->driving_dir = driving_dir_backward;
    }
}

void physics_update_rigid_body(physics_t* physics, vehicle_t* vehicle, vec3_t* pos, vec3_t* vel, vec3_t* nor)
{
    vec3_t floor_nor = *nor;
    vec3_norm(&floor_nor);
    float dot = vec3_dot(vel, &floor_nor);
    if (dot >= 0.0f)
        return;

    bool is_boosting = boost_type(&vehicle->boost) != boost_none;
    if (vehicle->floor.airtime > 20
        && !is_boosting
        && physics->vel.y < -50.0f)
    {
        pos->x = 0.0f;
        pos->z = 0.0f;
    }

    mat34_t mat34, tensor, transpose;
    mat33_t mat;
    vec3_t temp, temp2, cross, sum;

    float s = is_boosting ? 1.0f : 1.05f;
    float val, other_val;

    mat34_init_quat_pos(&mat34, &physics->main_rot, &vec3_zero);
    mat34_transpose(&mat34, &transpose);
    mat34_mulm(&mat34, &physics->inv_inertia_tensor, &tensor);
    mat34_mulm(&tensor, &transpose, &mat34);
    mat33_init_mat34(&mat, &mat34);

    vec3_cross(pos, &floor_nor, &temp);
    mat33_mulv(&mat, &temp, &cross);
    vec3_cross(&cross, pos, &temp);

    val = (-dot * s) / (1.0f + vec3_dot(&floor_nor, &temp));

    vec3_mul(vel, -1.0f, &temp2);
    vec3_cross(&floor_nor, &temp2, &cross);
    vec3_cross(&cross, &floor_nor, &temp);
    vec3_norm(&temp);

    other_val = val * vec3_dot(vel, &temp) / dot;
    other_val = copysignf(1.0f, other_val) * fminf(fabsf(other_val), 0.01f * val);

    vec3_mul(&floor_nor, val, &temp2);
    vec3_mul(&temp, other_val, &sum);
    vec3_add(&sum, &temp2, &sum);

    if (!vehicle->jump_pad.applied_dir
        && physics->vel1.y > 0.0f
        && physics->vel0.y + physics->vel1.y < 0.0)
    {
        physics->vel0.y += physics->vel1.y;
    }

    float last_vel0_y = physics->vel0.y;
    vec3_add(&physics->vel0, &sum, &physics->vel0);

    if (vehicle->jump_pad.applied_dir)
        physics->vel0.y = last_vel0_y;

    if (last_vel0_y < 0.0f && physics->vel0.y > 0.0 && physics->vel0.y < 10.0f)
        physics->vel0.y = 0.0f;

    vec3_cross(pos, &sum, &temp);
    mat33_mulv(&mat, &temp, &temp2);
    quat_inv_rotate(&physics->main_rot, &temp2, &cross);
    cross.y = 0.0f;
    vec3_add(&physics->rot_vec0, &cross, &physics->rot_vec0);
}

void physics_update_landing_angle(physics_t* physics)
{
	if (physics->landing_dir_valid)
	{
        vec3_t cross;
        float norm, dot, angle;

        vec3_cross(&physics->dir, &physics->landing_dir, &cross);
        norm  = vec3_magu(&cross);
        dot   = vec3_dot(&physics->dir, &physics->landing_dir);
        angle = degreesf(fabsf(atan2f(norm, dot)));
        dot   = vec3_dot(&cross, &physics->smoothed_up);
        physics->landing_angle += angle * copysignf(1.0f, dot);
	}

    if (physics->landing_angle < 0.0f)
        physics->landing_angle = fminf(physics->landing_angle + 2.0f, 0.0);
    else
        physics->landing_angle = fmaxf(physics->landing_angle - 2.0f, 0.0);
}

float physics_calc_acceleration(physics_t* physics, vehicle_t* vehicle)
{
    param_section_t* stats = &vehicle->stats;

    float acceleration;
    float t = physics->speed1 / physics->speed1_soft_limit;
    if (t >= 0.0f)
    {
        float *ys, *xs;
        int i, len;
        acceleration = 0.0f;

        if (vehicle->drift.state == drift_normal)
        {
            ys = stats->drift_acceleration_ys;
            xs = stats->drift_acceleration_xs;
            len = 2;
        }
        else
        {
            ys = stats->acceleration_ys;
            xs = stats->acceleration_xs;
            len = 4;
        }
    
        for (i = 1; i < len; i++)
        {
            if (t < xs[i])
            {
                acceleration = ys[i - 1] + (ys[i] - ys[i - 1]) / (xs[i] - xs[i - 1]) * (t - xs[i - 1]);
                break;
            }
        }
        if (i == len)
            acceleration = ys[len - 1];
    }
    else
    {
        acceleration = 1.0f;
    }

    return acceleration;
}

void physics_stabilize(physics_t* physics, vehicle_t* vehicle)
{
    vec3_t up, rot0_up;
    if (vehicle_is_bike(vehicle))
    {
        vec3_t front, right;
        quat_rotate(&physics->main_rot, &vec3_front, &front);
        vec3_cross(&physics->up, &front, &right);
        vec3_cross(&right, &physics->up, &front);
        vec3_norm(&front);

        float t;
        if (physics->speed1 >= 0.0f)
            t = fminf(physics->speed1_ratio * 2.0f, 1.0f);
        else
            t = 0.0f;

        vec3_t up_from, up_to, other_up;
        vec3_mul(&vec3_up, 1.0f - t, &up_to);
        vec3_mul(&physics->up, t, &up_from);
        vec3_add(&up_to, &up_from, &other_up);

        if (vec3_magsqr(&other_up) > FLT_EPSILON)
            vec3_norm(&other_up);
        else
            other_up = physics->up;

        vec3_cross(&other_up, &front, &right);
        vec3_cross(&front, &right, &up);
        vec3_norm(&up);
    }
    else
    {
        up = physics->up;
    }

    quat_rotate(&physics->main_rot, &vec3_up, &rot0_up);

    if (fabsf(vec3_dot(&up, &rot0_up)) < 0.9999f)
    {
        quat_t rot, rot0, rot_to;
        rot0 = physics->main_rot;
        quat_init_vecs(&rot, &rot0_up, &up);
        quat_mulq(&rot, &rot0, &rot_to);
        quat_slerp(&rot0, &rot_to, physics->stabilization_factor, &physics->main_rot);
    }
}