#pragma once

enum
{
	kmp_section_ktpt,
	kmp_section_enpt,
	kmp_section_enph,
	kmp_section_itpt,
	kmp_section_itph,
	kmp_section_ckpt,
	kmp_section_ckph,
	kmp_section_gobj,
	kmp_section_poti,
	kmp_section_area,
	kmp_section_came,
	kmp_section_jgpt,
	kmp_section_cnpt,
	kmp_section_mspi,
	kmp_section_stgi,

	kmp_section_max
};

typedef struct
{
	id_t		id;
	uint16_t	entry_count;
	uint16_t	extra;
} kmp_section_header_t;

typedef struct
{
	vec3_t		position;
	vec3_t		rotation;
	int16_t		index;
	uint16_t	pad;
} ktpt_t;

typedef struct
{
	vec3_t		position;
	float		threshold;
	uint16_t	setting1;
	uint8_t		setting2;
	uint8_t		setting3;
} enpt_t;

typedef struct
{
	uint8_t		start;
	uint8_t		length;
	uint8_t		prev[6];
	uint8_t		next[6];
	int16_t		link;
} enph_t;

typedef struct
{
	vec3_t		position;
	float		steer_factor;
	uint16_t	props1;
	uint16_t	props2;
} itpt_t;

typedef struct
{
	uint8_t		start;
	uint8_t		length;
	uint8_t		prev[6];
	uint8_t		next[6];
	uint16_t	pad;
} itph_t;

typedef struct
{
	vec2_t		left_point;
	vec2_t		right_point;
	uint8_t		respawn_index;
	int8_t		type;
	uint8_t		prev;
	uint8_t		next;
} ckpt_t;

typedef struct
{
	uint8_t		start;
	uint8_t		length;
	uint8_t		prev[6];
	uint8_t		next[6];
	uint16_t	pad;
} ckph_t;

typedef struct
{
	id_t		id;
	uint32_t	file_size;
	uint16_t	section_count;
	uint16_t	header_size;
	uint32_t	version;
	uint32_t	section_offsets[kmp_section_max];
} kmp_header_t;

enum
{
	checkpoint_normal = -1,
	checkpoint_finish = 0,
	/* checkpoint_key 1-127 */
};

typedef struct kmp_t
{
	kmp_section_header_t section_headers[kmp_section_max];

	union
	{
		struct
		{
			ktpt_t*		ktpt;
			enpt_t*		enpt;
			enph_t*		enph;
			itpt_t*		itpt;
			itph_t*		itph;
			ckpt_t*		ckpt;
			ckph_t*	    ckph;
			uint8_t*	gobj;
			uint8_t*	poti;
			uint8_t*	area;
			uint8_t*	came;
			uint8_t*	jgpt;
			uint8_t*	cnpt;
			uint8_t*	mspi;
			uint8_t*	stgi;
		};

		uint8_t*		sections[kmp_section_max];
	};
} kmp_t;

extern parser_t kmp_parser;