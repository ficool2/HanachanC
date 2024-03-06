#include "../common.h"

void bin_init(bin_t* bin)
{
	bin->buffer = NULL;
	bin->size = 0;
}

void bin_free(bin_t* bin)
{
	free(bin->buffer);
	bin->buffer = NULL;
	bin->size = 0;
}

void bin_set(bin_t* bin, uint8_t* buffer, size_t size)
{
	bin->buffer = buffer;
	bin->size = size;
}

int bin_read(bin_t* bin, const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (!file)
		return 0;

	fseek(file, 0, SEEK_END);
	bin->size = ftell(file);
	bin->buffer = malloc(bin->size);
	fseek(file, 0, SEEK_SET);
	fread(bin->buffer, 1, bin->size, file);
	fclose(file);
	return 1;
}

void bin_copy(bin_t* to, bin_t* from)
{
	to->buffer = malloc(from->size);
	to->size = from->size;
	memcpy(to->buffer, from->buffer, from->size);
}