#include "../common.h"
#include "floor.h"
#include "../vehicle/vehicle.h"
#include "../game/game.h"

#include "standstill_boost.h"

void standstill_boost_init(standstill_boost_t* boost)
{
	boost->rotation = 0.0f;
    boost->charge   = 0; 
}

void standstill_boost_update(standstill_boost_t* boost, vehicle_t* vehicle, int stage)
{
    float next = 0.0f;
    float t = 1.0f;
    if (vehicle->floor.airtime == 0)
    {
        if (stage == stage_countdown)
        {
            next = 0.015f * -vehicle->start_boost.charge;
        } 
        else if (!vehicle->standstill_miniturbo.charging &&
            vehicle->ramp_boost.duration == 0 && vehicle->jump_pad.variant == variant_invalid)
        {
            physics_t* physics = &vehicle->physics;
            float acceleration = clampf(physics->speed1 - physics->last_speed1, -3.0f, 3.0f);
            bool is_bike = vehicle_is_bike(vehicle);

            if (vehicle->boost.mushroom_boost > 0)
            {
                next = -acceleration * 0.15f * 0.25f;
                if (is_bike && vehicle->wheelie.is_wheelieing)
                    next *= 0.5f;
            }
            else
            {
                next = -acceleration * 0.15f * 0.08f;
            }

            t = is_bike ? 0.2f : 1.0f;
        }
        else
        {
            next = 0.015f * -((float)vehicle->standstill_miniturbo.charge / 75.0f);
        }
    }
    
    boost->rotation += t * (next - boost->rotation);
}