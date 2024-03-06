#include "../common.h"

void bswapstream_init(bswapstream_t* stream, uint8_t* buffer)
{
	stream->buffer = buffer;
	stream->pos = buffer;
}

int8_t bswapstream_read_int8(bswapstream_t* stream)
{
	int8_t v = *(int8_t*)stream->pos;
	stream->pos += sizeof(v);
	return v;
}

uint8_t bswapstream_read_uint8(bswapstream_t* stream)
{
	uint8_t v = *(uint8_t*)stream->pos;
	stream->pos += sizeof(v);
	return v;
}

int16_t bswapstream_read_int16(bswapstream_t* stream)
{
	int16_t v = bswap_int16(*(int16_t*)stream->pos);
	stream->pos += sizeof(v);
	return v;
}

uint16_t bswapstream_read_uint16(bswapstream_t* stream)
{
	uint16_t v = bswap_uint16(*(uint16_t*)stream->pos);
	stream->pos += sizeof(v);
	return v;
}

int32_t bswapstream_read_int32(bswapstream_t* stream)
{
	int32_t v = bswap_int32(*(int32_t*)stream->pos);
	stream->pos += sizeof(v);
	return v;
}

uint32_t bswapstream_read_uint32(bswapstream_t* stream)
{
	uint32_t v = bswap_uint32(*(uint32_t*)stream->pos);
	stream->pos += sizeof(v);
	return v;
}

float bswapstream_read_float(bswapstream_t* stream)
{
	float v = bswap_float(*(float*)stream->pos);
	stream->pos += sizeof(v);
	return v;
}

vec2_t bswapstream_read_vec2(bswapstream_t* stream)
{
	vec2_t v = *(vec2_t*)stream->pos;
	v = bswap_vec2(&v);
	stream->pos += sizeof(v);
	return v;
}

vec3_t bswapstream_read_vec3(bswapstream_t* stream)
{
	vec3_t v = *(vec3_t*)stream->pos;
	v = bswap_vec3(&v);
	stream->pos += sizeof(v);
	return v;
}

void bswapstream_seek(bswapstream_t* stream, size_t offset)
{
	stream->pos = stream->buffer + offset;
}

quat_t bswapstream_read_quat(bswapstream_t* stream)
{
	quat_t v = *(quat_t*)stream->pos;
	v = bswap_quat(&v);
	stream->pos += sizeof(v);
	return v;
}

id_t bswapstream_read_id(bswapstream_t* stream)
{
	id_t v = *(id_t*)stream->pos;
	/* intentionally not swapped */
	stream->pos += sizeof(v);
	return v;
}

void bswapstream_read_skip(bswapstream_t* stream, int bytes)
{
	stream->pos += bytes;
}

uint8_t* bswapstream_current_data(bswapstream_t* stream)
{
	return stream->pos;
}

size_t bswapstream_current_read(bswapstream_t* stream)
{
	return stream->pos - stream->buffer;
}

void bitstream_init(bitstream_t* stream, uint8_t* buffer)
{
	stream->buffer = buffer;
	stream->pos = 0;
}

#define bitstream_read(name, type) \
type name(bitstream_t* stream, int bits) \
{ \
	type value = 0; \
	for (int i = 0; i < bits; i++)  \
	{ \
		size_t byte_pos = stream->pos / 8; \
		size_t bit_pos = stream->pos % 8; \
		type bit = (stream->buffer[byte_pos] >> (7 - bit_pos)) & 1; \
		value = (value << 1) | bit; \
		stream->pos++; \
	} \
	return value; \
}

bitstream_read(bitstream_read_uint8, uint8_t)
bitstream_read(bitstream_read_uint16, uint16_t)
bitstream_read(bitstream_read_uint32, uint32_t)
#undef bitstream_read

void bitstream_read_skip(bitstream_t* stream, int bits)
{
	stream->pos += bits;
}

uint8_t* bitstream_current_data(bitstream_t* stream)
{
	return stream->buffer + stream->pos / 8;
}