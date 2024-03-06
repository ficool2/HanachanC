#pragma once

typedef struct
{
	char        cc[4];
} id_t;

typedef struct bin_t
{
    uint8_t*    buffer;
    size_t      size;
} bin_t;

void        bin_init(bin_t* bin);
void        bin_free(bin_t* bin);
void        bin_set (bin_t* bin, uint8_t* buffer, size_t size);
int         bin_read(bin_t* bin, const char* filename);
void        bin_copy(bin_t* to, bin_t* from);

typedef struct
{
	void	(*init )(void* obj);
	void	(*free )(void* obj);
	int		(*parse)(void* obj, bin_t* bin);
} parser_t;

typedef struct
{
	const char*	filename;
	void*		obj;
	parser_t*	parser;
	bin_t*		bin;
} parser_pair_t;

inline int parser_read(parser_t* parser, void* obj, const char* filename)
{
    bin_t buffer;
    int ret = 0;

	if (!bin_read(&buffer, filename))
	{
		printf("Couldn't open %s\n", filename);
		return ret;
	}

	parser->init(obj);
	if (parser->parse(obj, &buffer))
		ret = 1;
	else
		printf("Couldn't parse %s\n", filename);

	bin_free(&buffer);
	return ret;
}