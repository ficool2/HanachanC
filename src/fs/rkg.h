#pragma once

enum { rkg_max_size = 0x2774 };

typedef struct input_t input_t;

typedef struct
{
    uint8_t         minutes;
    uint8_t         seconds;
    uint16_t        milliseconds;
} rkg_time_t;

typedef struct 
{
    id_t            id;
    rkg_time_t      finish_time;
    uint8_t         course_id;
    uint8_t         vehicle_id;
    uint8_t         character_id;
    uint8_t         year;
    uint8_t         month;
    uint8_t         day;
    uint8_t         controller_id;
    uint8_t         compressed_flag;
    uint8_t         ghost_type;
    uint8_t         drift_type;
    uint16_t        input_data_length;
    uint8_t         lap_count;
    rkg_time_t      lap_split_times[5];
    uint8_t         country_code;
    uint8_t         state_code;
    uint16_t        location_code;
    uint32_t        unknown;
    uint8_t         mii_data[0x4A];
    uint16_t        crc16;
} rkg_header_t;

typedef struct
{
    uint16_t        face_button_count;
    uint16_t        direction_count;
    uint16_t        trick_count;
    uint16_t        unknown;
} rkg_input_header_t;

typedef struct rkg_t
{
    rkg_header_t        header;
    rkg_input_header_t  input_header;
    input_t*            frames;
    uint32_t            frame_count;
    uint32_t            crc32;
    char                name[_MAX_PATH];
} rkg_t;

extern parser_t rkg_parser;