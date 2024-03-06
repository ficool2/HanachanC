#include "../common.h"
#include "arc.h"
#include "yaz.h"

void arc_init(arc_t* arc)
{
	arc->nodes = NULL;
	arc->string_pool = NULL;
	arc->data = NULL;
	bin_init(&arc->bin);
}

void arc_free(arc_t* arc)
{
	free(arc->nodes);
	bin_free(&arc->bin);
	arc->nodes = NULL;
	arc->string_pool = NULL;
	arc->data = NULL;
}

int arc_parse(arc_t* arc, bin_t* arc_buffer)
{
	int compressed = yaz_is_compressed(arc_buffer);
	if (compressed)
	{
		bin_init(&arc->bin);
		if (yaz_decompress(arc_buffer, &arc->bin) != YAZ_OK)
		{
			printf("Error reading U8, failed to decompress\n");
			return 0;
		}
	}
	else
	{
		bin_copy(&arc->bin, arc_buffer);
	}

	arc_header_t* header = &arc->header;

	bswapstream_t stream;
	bswapstream_init(&stream, arc->bin.buffer);

	header->magic		= bswapstream_read_uint32(&stream);
	if (header->magic != 0x55AA382D)
	{
		printf("Error reading ARC, bad header\n");
		return 0;
	}

	header->root_offset = bswapstream_read_int32(&stream);
	header->size		= bswapstream_read_int32(&stream);
	header->data_offset	= bswapstream_read_int32(&stream);
	for (int i = 0; i < 4; i++)
		header->reserved[i] = bswapstream_read_int32(&stream);

	uint8_t* fs = bswapstream_current_data(&stream);
	bswapstream_seek(&stream, header->root_offset);

	/* root node */
	bswapstream_read_uint32(&stream);
	bswapstream_read_uint32(&stream);
	arc->node_count  = bswapstream_read_uint32(&stream);

	arc->nodes		 = malloc(arc->node_count * sizeof(arc_node_t));
	arc->string_pool = (char*)fs + arc->node_count * 0xC;
	arc->data        = arc->bin.buffer + arc->header.data_offset;
	arc->root        = &arc->nodes[0];

	bswapstream_seek(&stream, header->root_offset);
	for (uint32_t i = 0; i < arc->node_count; i++)
	{
		arc_node_t* node	     = &arc->nodes[i];

		uint32_t info			 = bswapstream_read_uint32(&stream);
		node->type				 = info >> 24;
		node->filename			 = arc->string_pool + (info & 0x00FFFFFF);
		bin_init(&node->data);

		if (node->type == arc_node_file)
		{
			uint32_t data_offset      = bswapstream_read_uint32(&stream);
			uint32_t data_size        = bswapstream_read_uint32(&stream);
			bin_set(&node->data, arc->bin.buffer + data_offset, data_size);
		}
		else if (node->type == arc_node_dir)
		{
			uint32_t parent_index	  = bswapstream_read_uint32(&stream);
			uint32_t first_node_index = bswapstream_read_uint32(&stream);
			node->parent_index	      = parent_index;
			node->last_index		  = first_node_index;
		}
		else
		{
			printf("Error reading ARC, bad node type %d at index %d\n", node->type, i);
			return 0;
		}
	}

	arc->start_index = 1;
	if (arc->node_count >= 2 && !strcmp(arc->nodes[1].filename, "."))
		arc->start_index = 2;

	return 1;
}

void arc_find(arc_t* arc, arc_find_callback callback, void* userdata)
{
	for (uint32_t i = arc->start_index; i < arc->node_count; i++)
	{
		arc_node_t* node = &arc->nodes[i];
		if (!callback(node, userdata))
			break;
	}
}

typedef struct
{
	const char* filename;
	bin_t*		data;
} arc_find_data_context_t;

int arc_find_data_callback(arc_node_t* node, void* userdata)
{
	if (node->type == arc_node_file)
	{
		arc_find_data_context_t* context = userdata;
		if (!strcmp(node->filename, context->filename))
		{
			context->data = &node->data;
			return 0;
		}
	}

	return 1;
}

bin_t* arc_find_data(arc_t* arc, const char* filename)
{
	arc_find_data_context_t context = { .filename = filename, .data = NULL };
	arc_find(arc, arc_find_data_callback, &context);
	return context.data;
}

int arc_print_callback(arc_node_t* node, void* userdata)
{
	char* path = (char*)userdata;
	if (node->type == arc_node_file)
	{
		printf("%08x %s%s\n", (uint32_t)node->data.size, path, node->filename);
	}
	else if (node->type == arc_node_dir)
	{
		/* TODO: pop directories */
		char* p = path;
		p = strcat2(p, node->filename);
		p = strcat2(p, "/");
	}

	return 1;
}

void arc_print(arc_t* arc)
{
	char path[_MAX_PATH] = { 0 };
	printf("Filesize Filename\n");
	arc_find(arc, arc_print_callback, path);
}

parser_t arc_parser =
{
	arc_init,
	arc_free,
	arc_parse,
};