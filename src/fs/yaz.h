#pragma once

enum
{
	YAZ_OK = 0,
	YAZ_INVALID_HEADER,
	YAZ_INVALID_SIZE,
};

typedef struct
{
	char        id[4];
	uint32_t    size_decompressed;
	uint32_t    pad[2];
} yaz_header_t;

int	yaz_is_compressed(bin_t* in);
int yaz_decompress   (bin_t* in, bin_t* out);