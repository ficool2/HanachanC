#include "../common.h"
#include "kmp.h"

typedef void* (*kmp_section_parser)(kmp_section_header_t* header, bswapstream_t* stream);

typedef struct
{
	const char*				name;
	kmp_section_parser		func;
} kmp_section_parse_t;

void* ktpt_parse(kmp_section_header_t* header, bswapstream_t* stream);
void* enpt_parse(kmp_section_header_t* header, bswapstream_t* stream);
void* enph_parse(kmp_section_header_t* header, bswapstream_t* stream);
void* itpt_parse(kmp_section_header_t* header, bswapstream_t* stream);
void* itph_parse(kmp_section_header_t* header, bswapstream_t* stream);
void* ckpt_parse(kmp_section_header_t* header, bswapstream_t* stream);
void* ckph_parse(kmp_section_header_t* header, bswapstream_t* stream);

kmp_section_parse_t kmp_section_parsers[] =
{
	{ "KTPT", ktpt_parse },
	{ "ENPT", enpt_parse },
	{ "ENPH", enph_parse },
	{ "ITPT", itpt_parse },
	{ "ITPH", itph_parse },
	{ "CKPT", ckpt_parse },
	{ "CKPH", ckph_parse },
	{ "GOBJ", NULL },
	{ "POTI", NULL },
	{ "AREA", NULL },
	{ "CAME", NULL },
	{ "JGPT", NULL },
	{ "CNPT", NULL },
	{ "MSPT", NULL },
	{ "STGI", NULL },
};

void kmp_init(kmp_t* kmp)
{
	memset(kmp->sections, 0, sizeof(kmp->sections));
}

int kmp_parse(kmp_t* kmp, bin_t* kmp_buffer)
{
	kmp_header_t header;

	bswapstream_t stream;
	bswapstream_init(&stream, kmp_buffer->buffer);

	header.id = bswapstream_read_id(&stream);
	if (strncmp(header.id.cc, "RKMD", 4))
	{
		printf("Error reading KMP, bad header\n");
		return 0;
	}

	header.file_size		= bswapstream_read_uint32(&stream);
	if (header.file_size != kmp_buffer->size)
	{
		printf("Error reading KMP, bad file size\n");
		return 0;
	}

	header.section_count	= bswapstream_read_uint16(&stream);
	if (header.section_count != kmp_section_max)
	{
		printf("Error reading KMP, bad section count\n");
		return 0;
	}

	header.header_size		= bswapstream_read_uint16(&stream);
	if (header.header_size != sizeof(kmp_header_t))
	{
		printf("Error reading KMP, bad header size\n");
		return 0;
	}

	header.version			= bswapstream_read_uint32(&stream);

	uint8_t* data = kmp_buffer->buffer + header.header_size;
	for (int i = 0; i < kmp_section_max; i++)
	{
		uint32_t offset = bswapstream_read_uint32(&stream);

		bswapstream_t section_stream;
		bswapstream_init(&section_stream, data + offset);

		kmp_section_header_t* section_header = &kmp->section_headers[i];
		section_header->id			= bswapstream_read_id(&section_stream);
		section_header->entry_count = bswapstream_read_uint16(&section_stream);
		section_header->extra		= bswapstream_read_uint16(&section_stream);

		kmp_section_parse_t* parser = &kmp_section_parsers[i];
		if (strncmp(parser->name, section_header->id.cc, 4))
		{
			printf("Error reading KMP, expected section %s but got %.4s at index %d\n", 
				parser->name, section_header->id.cc, i);
			continue;
		}

		if (parser->func)
			kmp->sections[i] = parser->func(section_header, &section_stream);
	}

	return 1;
}

void kmp_free(kmp_t* kmp)
{
	for (int i = 0; i < kmp_section_max; i++)
	{
		free(kmp->sections[i]);
		kmp->sections[i] = NULL;
	}
}

void* ktpt_parse(kmp_section_header_t* header, bswapstream_t* stream)
{
	ktpt_t* points = malloc(header->entry_count * sizeof(ktpt_t));
	for (uint16_t i = 0; i < header->entry_count; i++)
	{
		ktpt_t* point			  = &points[i];
		point->position			  = bswapstream_read_vec3(stream);
		point->rotation			  = bswapstream_read_vec3(stream);
		point->index			  = bswapstream_read_int16(stream);
		point->pad				  = bswapstream_read_uint16(stream);
	}
	return points;
}

void* enpt_parse(kmp_section_header_t* header, bswapstream_t* stream)
{
	enpt_t* points = malloc(header->entry_count * sizeof(enpt_t));
	for (uint16_t i = 0; i < header->entry_count; i++)
	{
		enpt_t* point			  = &points[i];
		point->position			  = bswapstream_read_vec3(stream);
		point->threshold		  = bswapstream_read_float(stream);
		point->setting1			  = bswapstream_read_uint16(stream);
		point->setting2			  = bswapstream_read_uint8(stream);
		point->setting3			  = bswapstream_read_uint8(stream);
	}
	return points;
}

void* enph_parse(kmp_section_header_t* header, bswapstream_t* stream)
{
	enph_t* paths = malloc(header->entry_count * sizeof(enph_t));
	for (uint16_t i = 0; i < header->entry_count; i++)
	{
		enph_t* path			 = &paths[i];
		path->start				 = bswapstream_read_uint8(stream);
		path->length			 = bswapstream_read_uint8(stream);
		for (int j = 0; j < 6; j++)
			path->prev[j]		 = bswapstream_read_uint8(stream);
		for (int j = 0; j < 6; j++)
			path->next[j]		 = bswapstream_read_uint8(stream);
		path->link			     = bswapstream_read_int16(stream);
	}
	return paths;
}

void* itpt_parse(kmp_section_header_t* header, bswapstream_t* stream)
{
	itpt_t* points = malloc(header->entry_count * sizeof(itpt_t));
	for (uint16_t i = 0; i < header->entry_count; i++)
	{
		itpt_t* point			 = &points[i];
		point->position			 = bswapstream_read_vec3(stream);
		point->steer_factor		 = bswapstream_read_float(stream);
		point->props1			 = bswapstream_read_uint16(stream);
		point->props2			 = bswapstream_read_uint16(stream);
	}
	return points;
}

void* itph_parse(kmp_section_header_t* header, bswapstream_t* stream)
{
	itph_t* paths = malloc(header->entry_count * sizeof(itph_t));
	for (uint16_t i = 0; i < header->entry_count; i++)
	{
		itph_t* path			 = &paths[i];
		path->start				 = bswapstream_read_uint8(stream);
		path->length			 = bswapstream_read_uint8(stream);
		for (int j = 0; j < 6; j++)
			path->prev[j]		 = bswapstream_read_uint8(stream);
		for (int j = 0; j < 6; j++)
			path->next[j]		 = bswapstream_read_uint8(stream);
		path->pad			     = bswapstream_read_int16(stream);
	}
	return paths;
}

void* ckpt_parse(kmp_section_header_t* header, bswapstream_t* stream)
{
	ckpt_t* checkpoints = malloc(header->entry_count * sizeof(ckpt_t));
	for (uint16_t i = 0; i < header->entry_count; i++)
	{
		ckpt_t* checkpoint		  = &checkpoints[i];
		checkpoint->left_point    = bswapstream_read_vec2(stream);
		checkpoint->right_point   = bswapstream_read_vec2(stream);
		checkpoint->respawn_index = bswapstream_read_uint8(stream);
		checkpoint->type          = bswapstream_read_int8(stream);
		checkpoint->prev          = bswapstream_read_uint8(stream);
		checkpoint->next          = bswapstream_read_uint8(stream);
	}
	return checkpoints;
}

void* ckph_parse(kmp_section_header_t* header, bswapstream_t* stream)
{
	ckph_t* paths = malloc(header->entry_count * sizeof(ckph_t));
	for (uint16_t i = 0; i < header->entry_count; i++)
	{
		ckph_t* path			 = &paths[i];
		path->start				 = bswapstream_read_uint8(stream);
		path->length			 = bswapstream_read_uint8(stream);
		for (int j = 0; j < 6; j++)
			path->prev[j]		 = bswapstream_read_uint8(stream);
		for (int j = 0; j < 6; j++)
			path->next[j]		 = bswapstream_read_uint8(stream);
		path->pad			     = bswapstream_read_int16(stream);
	}
	return paths;
}

parser_t kmp_parser =
{
	kmp_init,
	kmp_free,
	kmp_parse,
};