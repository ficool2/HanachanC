#pragma once

enum 
{
	COL_TYPE_ROAD = 0,
	COL_TYPE_SLIPPERY_ROAD = 1,
	COL_TYPE_WEAK_OFF_ROAD = 2,
	COL_TYPE_OFF_ROAD = 3,
	COL_TYPE_HEAVY_OFF_ROAD = 4,
	COL_TYPE_SLIPPERY_ROAD_2 = 5,
	COL_TYPE_BOOST_PAD = 6,
	COL_TYPE_BOOST_RAMP = 7,
	COL_TYPE_JUMP_PAD = 8,
	COL_TYPE_ITEM_ROAD = 9,

	COL_TYPE_SOLID_OOB = 0xa,
	COL_TYPE_MOVING_WATER = 0xb,

	COL_TYPE_WALL = 0xc,
	COL_TYPE_INVISIBLE_WALL = 0xd,
	COL_TYPE_ITEM_WALL = 0xe,
	COL_TYPE_WALL_2 = 0xf,

	COL_TYPE_FALL_BOUNDARY = 0x10,
	COL_TYPE_CANNON_TRIGGER = 0x11,
	COL_TYPE_FORCE_RECALCULATE_ROUTE = 0x12,
	COL_TYPE_HALFPIPE_RAMP = 0x13,
	COL_TYPE_PLAYER_WALL = 0x14,
	COL_TYPE_MOVING_ROAD = 0x15,
	COL_TYPE_STICKY_ROAD = 0x16,
	COL_TYPE_ROAD2 = 0x17,
	COL_TYPE_SOUND_TRIGGER = 0x18,
	COL_TYPE_WEAK_WALL = 0x19,
	COL_TYPE_EFFECT_TRIGGER = 0x1a,
	COL_TYPE_ITEM_STATE_MODIFIER = 0x1b,

	COL_TYPE_HALFPIPE_INVISIBLE_WALL = 0x1c,
	COL_TYPE_ROTATING_ROAD = 0x1d,
	COL_TYPE_SPECIAL_WALL = 0x1e,
	COL_TYPE_INVISIBLE_WALL2 = 0x1f,

	COL_TYPE_COUNT
};

#define KCL_ATTRIBUTE_TYPE(x) ((x) & 0x1f)
#define KCL_TYPE_BIT(x) (1 << (x))
#define KCL_ATTRIBUTE_TYPE_BIT(x) KCL_TYPE_BIT(KCL_ATTRIBUTE_TYPE(x))

#define KCL_TYPE_DIRECTIONAL (\
    KCL_TYPE_BIT(COL_TYPE_FALL_BOUNDARY) | \
    KCL_TYPE_BIT(COL_TYPE_SOUND_TRIGGER) | \
    KCL_TYPE_BIT(COL_TYPE_FORCE_RECALCULATE_ROUTE) | \
    KCL_TYPE_BIT(COL_TYPE_EFFECT_TRIGGER) | \
    KCL_TYPE_BIT(COL_TYPE_CANNON_TRIGGER))

#define KCL_TYPE_SOLID_SURFACE ( 0xffffffff & (\
    ~KCL_TYPE_BIT(COL_TYPE_FALL_BOUNDARY) & \
    ~KCL_TYPE_BIT(COL_TYPE_CANNON_TRIGGER) & \
    ~KCL_TYPE_BIT(COL_TYPE_FORCE_RECALCULATE_ROUTE) & \
    ~KCL_TYPE_BIT(COL_TYPE_SOUND_TRIGGER) & \
    ~KCL_TYPE_BIT(COL_TYPE_WEAK_WALL) & \
    ~KCL_TYPE_BIT(COL_TYPE_EFFECT_TRIGGER) & \
    ~KCL_TYPE_BIT(COL_TYPE_ITEM_STATE_MODIFIER)))

#define KCL_TYPE_FLOOR (\
    KCL_TYPE_BIT(COL_TYPE_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_SLIPPERY_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_WEAK_OFF_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_OFF_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_HEAVY_OFF_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_SLIPPERY_ROAD_2) | \
    KCL_TYPE_BIT(COL_TYPE_BOOST_PAD) | \
    KCL_TYPE_BIT(COL_TYPE_BOOST_RAMP) | \
    KCL_TYPE_BIT(COL_TYPE_JUMP_PAD) | \
    KCL_TYPE_BIT(COL_TYPE_ITEM_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_SOLID_OOB) | \
    KCL_TYPE_BIT(COL_TYPE_MOVING_WATER) | \
    KCL_TYPE_BIT(COL_TYPE_HALFPIPE_RAMP) | \
    KCL_TYPE_BIT(COL_TYPE_MOVING_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_STICKY_ROAD) | \
    KCL_TYPE_BIT(COL_TYPE_ROAD2) | \
    KCL_TYPE_BIT(COL_TYPE_ROTATING_ROAD))

#define KCL_TYPE_WALL (\
    KCL_TYPE_BIT(COL_TYPE_WALL) | \
    KCL_TYPE_BIT(COL_TYPE_INVISIBLE_WALL) | \
    KCL_TYPE_BIT(COL_TYPE_ITEM_WALL) | \
    KCL_TYPE_BIT(COL_TYPE_WALL_2 ) | \
    KCL_TYPE_BIT(COL_TYPE_PLAYER_WALL) | \
    KCL_TYPE_BIT(COL_TYPE_HALFPIPE_INVISIBLE_WALL) | \
    KCL_TYPE_BIT(COL_TYPE_SPECIAL_WALL) | \
    KCL_TYPE_BIT(COL_TYPE_INVISIBLE_WALL2))

#define KCL_SOFT_WALL_MASK 0x8000

extern const vec3_t kcl_flag_colors[];

typedef struct bsp_hitbox_t bsp_hitbox_t;

enum
{
	variant_invalid = 0xFF,
};

float variant_jump_pad_speed(uint8_t variant);
float variant_jump_pad_vel_y(uint8_t variant);

typedef struct
{
	float		dist;
	vec3_t		normal;
	uint16_t	flags;
} collision_tri_t;

typedef struct
{
	uint16_t	surface;
	float		dist;
} hit_t;

enum { collision_max_hits = 64 };

typedef struct collision_t
{
	vec3_t		min;
	vec3_t		max;
	float		floor_dist;
	vec3_t		floor_normal;
	hit_t		hits[collision_max_hits];
	int			hit_count;
	uint32_t	surface_kinds;
} collision_t;

void   collision_init(collision_t* collision);
void   collision_add_tri(collision_t* collision, collision_tri_t* tri);
void   collision_movement(collision_t* collision, vec3_t* out);
hit_t* collision_find_furthest(collision_t* collision, uint32_t surface_kinds);

typedef struct hitbox_t
{
	vec3_t		pos;
	vec3_t		last_pos;
	float		radius;
	uint32_t	flags;
	bool		last_pos_valid;
} hitbox_t;

void hitbox_init(hitbox_t* hitbox, vec3_t* pos, vec3_t* last_pos, float radius, uint32_t flags);
void hitbox_update_pos(hitbox_t* hitbox, vec3_t* pos);

typedef struct
{
	uint32_t	pos_data_offset;
	uint32_t	nrm_data_offset;
	uint32_t	tri_data_offset;
	uint32_t	octree_data_offset;
	float		thickness;
	vec3_t		origin;
	uint32_t	x_mask;
	uint32_t	y_mask;
	uint32_t	z_mask;
	uint32_t	shift;
	uint32_t	y_shift;
	uint32_t	z_shift;
	float		sphere_radius;
} kcl_header_t;

typedef struct
{
	uint8_t		type;
	uint32_t	idx;
} kcl_node_t;

typedef struct
{
	kcl_node_t	nodes[8];
} kcl_branch_t;

typedef struct
{
	uint16_t*	tris;
	uint32_t	tri_count;
} kcl_tri_list_t;

typedef struct
{
	kcl_node_t*		root_nodes;
	uint32_t		root_node_count;
	kcl_branch_t*	branches;
	uint32_t		branch_count;
	kcl_tri_list_t* tri_lists;
	uint32_t		tri_list_count;
} kcl_octree_t;

typedef struct
{
	float		height;
	vec3_t		position;
	vec3_t		normal;
	vec3_t		ca_normal;
	vec3_t		ab_normal;
	vec3_t		bc_normal;
	uint16_t	attribute;
} kcl_tri_t;

typedef struct
{
	vec3_t		position;
	vec3_t		normal;
	vec3_t		color;
} kcl_tri_vertex_t;

typedef struct kcl_t
{
	kcl_header_t header;
	kcl_tri_t*   tris;
	kcl_tri_vertex_t* vertices;
	uint32_t	 tri_count;
	uint32_t	 vertex_count;
	kcl_octree_t octree;
} kcl_t;

void kcl_collision_hitbox(kcl_t* kcl, hitbox_t* hitbox, collision_t* collision);
void kcl_write_obj(kcl_t* kcl, const char* name);

extern parser_t kcl_parser;