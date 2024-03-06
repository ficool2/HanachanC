#pragma once

typedef struct
{
	uint32_t		magic;
	int32_t			root_offset;
	int32_t			size;
	int32_t			data_offset;
	int32_t			reserved[4];
} arc_header_t;

enum
{
	arc_node_file = 0x00,
	arc_node_dir  = 0x01
};

typedef struct
{
	uint8_t			type;
	char*			filename;
	bin_t			data;
	uint32_t		parent_index;
	uint32_t		last_index;
} arc_node_t;

typedef struct arc_t
{
	arc_header_t	header;
	arc_node_t*		nodes;
	arc_node_t*		root;
	uint32_t		start_index;
	uint32_t		node_count;
	char*			string_pool;
	uint8_t*		data;
	bin_t			bin;
} arc_t;

extern parser_t arc_parser;

typedef int (*arc_find_callback)(arc_node_t* node, void* userdata);

void    arc_find (arc_t* arc, arc_find_callback callback, void* userdata);
bin_t*	arc_find_data(arc_t* arc, const char* filename);
void	arc_print(arc_t* arc);