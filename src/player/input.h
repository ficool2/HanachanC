#pragma once

typedef struct input_t
{
    bool        accelerate;
    bool        brake;
    bool        use_item;
    bool        drift;
    float       stick_x;
    float       stick_y;
    uint8_t     trick;
} input_t;