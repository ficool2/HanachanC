#pragma once

typedef struct
{
    uint8_t*    buffer;
    uint8_t*    pos;
} bswapstream_t;

void        bswapstream_init(bswapstream_t* stream, uint8_t* buffer);
int8_t      bswapstream_read_int8(bswapstream_t* stream);
uint8_t     bswapstream_read_uint8(bswapstream_t* stream);
int16_t     bswapstream_read_int16(bswapstream_t* stream);
uint16_t    bswapstream_read_uint16(bswapstream_t* stream);
int32_t     bswapstream_read_int32(bswapstream_t* stream);
uint32_t    bswapstream_read_uint32(bswapstream_t* stream);
float       bswapstream_read_float(bswapstream_t* stream);
vec2_t      bswapstream_read_vec2(bswapstream_t* stream);
vec3_t      bswapstream_read_vec3(bswapstream_t* stream);
quat_t      bswapstream_read_quat(bswapstream_t* stream);
id_t        bswapstream_read_id(bswapstream_t* stream);
void        bswapstream_read_skip(bswapstream_t* stream, int bytes);
void        bswapstream_seek(bswapstream_t*, size_t offset);
uint8_t*    bswapstream_current_data(bswapstream_t* stream);
size_t      bswapstream_current_read(bswapstream_t* stream);

typedef struct bitstream_t
{
    uint8_t*    buffer;
    int         pos;
} bitstream_t;

void        bitstream_init(bitstream_t* stream, uint8_t* buffer);
uint8_t     bitstream_read_uint8(bitstream_t* stream, int bits);
uint16_t    bitstream_read_uint16(bitstream_t* stream, int bits);
uint32_t    bitstream_read_uint32(bitstream_t* stream, int bits);
void        bitstream_read_skip(bitstream_t* stream, int bits);
uint8_t*    bitstream_current_data(bitstream_t*);