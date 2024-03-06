#include "../common.h"
#include "kcl.h"
#include <float.h>

enum
{
	kcl_node_leaf,
	kcl_node_branch,
};

typedef struct
{
	uint8_t  type;
	uint32_t offset;
} kcl_raw_node_t;

typedef struct
{
	kcl_raw_node_t nodes[8];
} kcl_raw_branch_t;

typedef struct
{
	uint16_t* tris;
	uint32_t  tri_count;
	uint32_t  offset;
} kcl_raw_tri_list_t;

const vec3_t kcl_flag_colors[] =
{
    { 0.750f, 0.750f, 0.750f },  /* Road (0x00)                     */
    { 1.000f, 1.000f, 1.000f },  /* Slippery Road 1 (0x01)          */
    { 0.271f, 0.545f, 0.075f },  /* Weak Off-road (0x02)            */
    { 0.322f, 0.627f, 0.176f },  /* Off-road (0x03)                 */
    { 0.412f, 0.824f, 0.118f },  /* Heavy Off-road (0x04)           */
    { 0.961f, 0.961f, 0.863f },  /* Slippery Road 2 (0x05)          */
    { 1.000f, 0.843f, 0.000f },  /* Boost (0x06) [DASH]             */
    { 1.000f, 0.875f, 0.000f },  /* Boost Ramp (0x07) [DASHJ]       */
    { 0.529f, 0.808f, 0.922f },  /* Jump Pad (0x08)                 */
    { 1.000f, 0.647f, 0.000f },  /* Item Road (0x09)                */
    { 0.545f, 0.000f, 0.000f },  /* Solid Fall (0x0A)               */
    { 0.000f, 0.000f, 1.000f },  /* Moving Water (0x0B)             */
    { 0.300f, 0.300f, 0.750f },  /* Wall (0x0C)                     */
    { 0.412f, 0.412f, 0.412f },  /* Invisible Wall (0x0D)           */
    { 1.000f, 0.549f, 0.000f },  /* Item Wall (0x0E)                */
    { 0.439f, 0.502f, 0.565f },  /* Wall 2 (0x0F)                   */
    { 1.000f, 0.000f, 0.000f },  /* Fall Boundary (0x10)            */
    { 1.000f, 0.078f, 0.576f },  /* Cannon Trigger (0x11)           */
    { 1.000f, 0.412f, 0.706f },  /* Force Recalculation (0x12)      */
    { 1.000f, 0.753f, 0.796f },  /* Half-pipe Ramp (0x13)           */
    { 0.863f, 0.078f, 0.235f },  /* Player-Only Wall (0x14)         */
    { 1.000f, 0.627f, 0.478f },  /* Moving Road (0x15)              */
    { 0.914f, 0.588f, 0.478f },  /* Sticky Road (0x16) [ATTACH]     */
    { 0.941f, 0.502f, 0.502f },  /* Road 2 (0x17)                   */
    { 1.000f, 0.388f, 0.278f },  /* Sound Trigger (0x18)            */
    { 0.804f, 0.361f, 0.361f },  /* Weak Wall (0x19)                */
    { 0.698f, 0.133f, 0.133f },  /* Effect Trigger (0x1A)           */
    { 1.000f, 0.000f, 0.000f },  /* Item State Modifier (0x1B)      */
    { 0.545f, 0.000f, 0.000f },  /* Half-Pipe Invisible Wall (0x1C) */
    { 0.502f, 0.000f, 0.000f },  /* Rotating Road (0x1D)            */
    { 1.000f, 0.000f, 1.000f },  /* Special Wall (0x1E)             */
    { 0.933f, 0.510f, 0.933f },  /* Invisible Wall 2 (0x1F)         */
};

float variant_jump_pad_speed(uint8_t variant)
{
	switch (variant)
	{
		case 0: return 50.0f;
		case 1: return 50.0f;
		case 2: return 59.0f;
		case 3: return 73.0f;
		case 4: return 73.0f;
		case 5: return 56.0f;
		case 6: return 55.0f;
		case 7: return 56.0f;
	}

	return 0.0f;
}

float variant_jump_pad_vel_y(uint8_t variant)
{
	switch (variant)
	{
		case 0: return 35.0f;
		case 1: return 47.0f;
		case 2: return 30.0f;
		case 3: return 45.0f;
		case 4: return 53.0f;
		case 5: return 50.0f;
		case 6: return 35.0f;
		case 7: return 50.0f;
	}

	return 0.0f;
}

void collision_init(collision_t* collision)
{
	memset(collision, 0, sizeof(*collision));
}

void collision_add_tri(collision_t* collision, collision_tri_t* tri)
{
	vec3_t dir;
	vec3_mul(&tri->normal, tri->dist, &dir);

	vec3_min(&collision->min, &dir, &collision->min);
	vec3_max(&collision->max, &dir, &collision->max);

	if (tri->dist > collision->floor_dist)
	{
		collision->floor_dist	= tri->dist;
		collision->floor_normal = tri->normal;
	}

	collision->surface_kinds |= 1 << (tri->flags & 0x1F);

	if (collision->hit_count < collision_max_hits)
	{
		hit_t* hit   = &collision->hits[collision->hit_count++];
		hit->surface = tri->flags;
		hit->dist    = tri->dist;
	}
}

void collision_movement(collision_t* collision, vec3_t* out)
{
	vec3_add(&collision->min, &collision->max, out);
}

hit_t* collision_find_furthest(collision_t* collision, uint32_t surface_kinds)
{
	hit_t* closest = NULL;
	float closest_dist = -FLT_MAX;

	for (int i = 0; i < collision->hit_count; i++)
	{
		hit_t* hit = &collision->hits[i];
		if (((1 << (hit->surface & 0x1F)) & surface_kinds) == 0)
			continue;

		if (hit->dist > closest_dist)
		{
			closest = hit;
			closest_dist = hit->dist;
		}
	}

	return closest;
}


void hitbox_init(hitbox_t* hitbox, vec3_t* pos, vec3_t* last_pos, float radius, uint32_t flags)
{
	hitbox->pos = *pos;
	hitbox->last_pos = last_pos ? *last_pos : vec3_zero;
	hitbox->radius = radius;
	hitbox->flags = flags;
	hitbox->last_pos_valid = last_pos != NULL;
}

void hitbox_update_pos(hitbox_t* hitbox, vec3_t* pos)
{
	hitbox->last_pos = hitbox->pos;
	hitbox->pos      = *pos;
	hitbox->last_pos_valid = true;
}

float kcl_tri_dot(vec3_t* a, vec3_t* b)
{
	float y = a->y * b->y;
	float xy = (float)(((double)a->x * (double)b->x) + (double)y);
	return xy + a->z * b->z;
}

bool kcl_tri_collision_hitbox(kcl_tri_t* tri, hitbox_t* hitbox, float thickness, collision_tri_t* collision)
{
	if ((1 << (tri->attribute & 0x1F) & hitbox->flags) == 0)
		return false;

	vec3_t pos;
	float radius = hitbox->radius;
	vec3_sub(&hitbox->pos, &tri->position, &pos);

	float ca_dist = kcl_tri_dot(&pos, &tri->ca_normal);
	if (ca_dist >= radius)
		return false;

	float ab_dist = kcl_tri_dot(&pos, &tri->ab_normal);
	if (ab_dist >= radius)
		return false;

	float bc_dist = kcl_tri_dot(&pos, &tri->bc_normal) - tri->height;
	if (bc_dist >= radius)
		return false;

	float plane_dist = kcl_tri_dot(&pos, &tri->normal);
	float dist_in_plane = radius - plane_dist;
	if (dist_in_plane <= 0.0f || dist_in_plane >= thickness)
		return false;

	if (ca_dist <= 0.0f && ab_dist <= 0.0 && bc_dist <= 0.0)
	{
		if (hitbox->last_pos_valid)
		{
			vec3_t last_pos;
			vec3_sub(&hitbox->pos, &hitbox->last_pos, &last_pos);
			vec3_sub(&pos, &last_pos, &last_pos);

			if (plane_dist < 0.0 && kcl_tri_dot(&last_pos, &tri->normal) < 0.0f)
				return false;
		}

		collision->dist   = dist_in_plane;
		collision->normal = tri->normal;
		collision->flags  = tri->attribute;
		return true;
	}

	vec3_t edge_nor, other_edge_nor;
	float edge_dist, other_edge_dist;
	if (ab_dist >= ca_dist && ab_dist > bc_dist)
	{
		edge_nor = tri->ab_normal;
		edge_dist = ab_dist;
		if (ca_dist >= bc_dist)
		{
			other_edge_nor = tri->ca_normal;
			other_edge_dist = ca_dist;
		}
		else
		{
			other_edge_nor = tri->bc_normal;
			other_edge_dist = bc_dist;
		}
	}
	else if (bc_dist >= ca_dist)
	{
		edge_nor = tri->bc_normal;
		edge_dist = bc_dist;
		if (ab_dist >= ca_dist)
		{
			other_edge_nor = tri->ab_normal;
			other_edge_dist = ab_dist;
		}
		else 
		{
			other_edge_nor = tri->ca_normal;
			other_edge_dist = ca_dist;
		}
	}
	else
	{
		edge_nor = tri->ca_normal;
		edge_dist = ca_dist;
		if (bc_dist >= ab_dist)
		{
			other_edge_nor = tri->bc_normal;
			other_edge_dist = bc_dist;
		}
		else
		{
			other_edge_nor = tri->ab_normal;
			other_edge_dist = ab_dist;
		}
	}

	float c = kcl_tri_dot(&edge_nor, &other_edge_nor);
	float sq_dist;

	if (c * edge_dist > other_edge_dist)
	{
		if (!hitbox->last_pos_valid && edge_dist >= plane_dist)
			return false;

		sq_dist = radius * radius - edge_dist * edge_dist;
	}
	else
	{
		float t = (c * edge_dist - other_edge_dist) / (c * c - 1.0f);
		float s = edge_dist - t * c;
		vec3_t corner1, corner2, corner_pos;
		vec3_mul(&edge_nor, s, &corner1);
		vec3_mul(&other_edge_nor, t, &corner2);
		vec3_add(&corner1, &corner2, &corner_pos);
		float corner_sq_dist = vec3_magsqr(&corner_pos);

		if (!hitbox->last_pos_valid && corner_sq_dist > plane_dist * plane_dist)
			return false;

		sq_dist = radius * radius - corner_sq_dist;
	}

	if (sq_dist < plane_dist * plane_dist || sq_dist < 0.0f)
		return false;

	float dist = sqrtf(sq_dist) - plane_dist;
	if (dist <= 0.0f)
		return false;

	if (hitbox->last_pos_valid)
	{
		vec3_t last_pos;
		vec3_sub(&hitbox->pos, &hitbox->last_pos, &last_pos);
		vec3_sub(&pos, &last_pos, &last_pos);

		if (kcl_tri_dot(&last_pos, &tri->normal) < 0.0f)
			return false;
	}

	collision->dist   = dist;
	collision->normal = tri->normal;
	collision->flags  = tri->attribute;
	return true;
}

void kcl_octree_init(kcl_octree_t* octree)
{
	octree->root_nodes = NULL;
	octree->root_node_count = 0;
	octree->branches = NULL;
	octree->branch_count = 0;
	octree->tri_lists = NULL;
	octree->tri_list_count = 0;
}

void kcl_octree_free(kcl_octree_t* octree)
{
	free(octree->root_nodes);
	free(octree->branches);
	for (uint32_t i = 0; i < octree->tri_list_count; i++)
		free(octree->tri_lists[i].tris);
	free(octree->tri_lists);

	octree->root_nodes = NULL;
	octree->root_node_count = 0;
	octree->branches = NULL;
	octree->branch_count = 0;
	octree->tri_lists = NULL;
	octree->tri_list_count = 0;
}

bool kcl_octree_parse_node(kcl_raw_node_t* node, bswapstream_t* stream, 
	uint32_t* nodes_size, uint32_t* tri_lists_offset, uint32_t parent_offset)
{
	uint32_t offset = bswapstream_read_uint32(stream);
	uint32_t leaf = offset & 0x80000000;
	offset = (offset & ~0x80000000) + parent_offset;
	if (leaf)
	{
		offset += 2;
		*tri_lists_offset = min(*tri_lists_offset, offset);
		node->type   = kcl_node_leaf;
		node->offset = offset;
		return true;
	}
	else if (offset % 4 == 0)
	{
		*nodes_size  = max(*nodes_size, offset + 32);
		node->type   = kcl_node_branch;
		node->offset = offset;
		return true;
	}
	else
	{
		return false;
	}
}

bool kcl_node_from_raw(kcl_node_t* node, kcl_raw_node_t* raw_node, 
	uint32_t branches_offset, kcl_raw_tri_list_t* tri_lists, uint32_t tri_list_count)
{
	node->type = raw_node->type;
	if (node->type == kcl_node_leaf)
	{
		uint32_t j;
		for (j = 0; j < tri_list_count; j++)
			if (tri_lists[j].offset == raw_node->offset)
				break;
	
		if (j == tri_list_count)
		{
			printf("Error: Failed to match node to tri list in KCL\n");
			return false;
		}
	
		node->idx = j;
	}
	else if (node->type == kcl_node_branch)
	{
		node->idx = (raw_node->offset - branches_offset) / 32;
	}

	return true;
}

int kcl_octree_parse(kcl_octree_t* octree, bswapstream_t* stream, uint32_t size)
{
	int ret = 0;
	uint8_t* octree_start = bswapstream_current_data(stream);

	uint32_t nodes_size = 0;
	uint32_t tri_lists_offset = size;
	uint32_t parent_offset = 0;

	uint32_t root_node_count = size / 4;
	kcl_raw_node_t* root_nodes = malloc(root_node_count * sizeof(kcl_raw_node_t));

	for (uint32_t i = 0; i < root_node_count; i++)
	{
		kcl_raw_node_t* node = &root_nodes[i];
		if (!kcl_octree_parse_node(node, stream, &nodes_size, &tri_lists_offset, parent_offset))
		{
			printf("Error parsing KCL octree node at index %u\n", i);
			free(root_nodes);
			return ret;
		}

		root_node_count = min(root_node_count, node->offset / 4);
	}

	uint32_t branches_offset = root_node_count * 4;
	nodes_size = max(nodes_size, branches_offset);

	uint32_t branch_count = 0;
	/* TODO proper size calculation */
	kcl_raw_branch_t* branches = malloc(size / 8 * sizeof(kcl_raw_branch_t));

	for (;;)
	{
		if (branch_count >= (nodes_size - branches_offset) / 32)
			break;

		uint32_t branch_offset = branches_offset + branch_count * 32;
		kcl_raw_node_t* nodes = branches[branch_count].nodes;
		for (int i = 0; i < 8; i++)
		{
			if (!kcl_octree_parse_node(&nodes[i], stream, 
				&nodes_size, &tri_lists_offset, branch_offset))
			{
				free(root_nodes);
				free(branches);
				return ret;
			}
		}

		branch_count++;
	}

	if (nodes_size != tri_lists_offset)
	{
		printf("Error parsing KCL octree, %u != %u\n", nodes_size, tri_lists_offset);
		free(root_nodes);
		free(branches);
		return ret;
	}

	uint32_t tri_index_size = (uint32_t)((octree_start + size) - bswapstream_current_data(stream));
	uint32_t tri_index_count = tri_index_size / sizeof(uint16_t);
	uint16_t* tri_index = malloc(tri_index_count * sizeof(uint16_t));
	uint32_t tri_list_count = 0;

	for (uint32_t i = 0; i < tri_index_count; i++)
	{
		uint16_t index = bswapstream_read_uint16(stream);
		if (index == 0)
		{
			tri_list_count++;
			tri_index[i] = 0xFFFF;
		}
		else
		{
			tri_index[i] = index - 1;
		}
	}

	kcl_raw_tri_list_t* tri_lists = malloc(sizeof(kcl_raw_tri_list_t) * (tri_list_count + 1));
	uint32_t tri_list_start_index = 0;
	tri_list_count = 0;

	for (uint32_t i = 0; i < tri_index_count; i++)
	{
		uint16_t index = tri_index[i];
		if (index == 0xFFFF)
		{
			kcl_raw_tri_list_t* tri_list = &tri_lists[tri_list_count++];

			tri_list->tri_count = i - tri_list_start_index;
			tri_list->tris = malloc(tri_list->tri_count * sizeof(*tri_list->tris));
			for (uint32_t j = 0; j < tri_list->tri_count; j++)
				tri_list->tris[j] = tri_index[tri_list_start_index + j];
			tri_list->offset = tri_lists_offset;

			tri_list_start_index = i + 1;
			tri_lists_offset += (tri_list->tri_count + 1) * 2;
		}
	}

	free(tri_index);

	kcl_raw_tri_list_t* last_tri_list = &tri_lists[tri_list_count++];
	last_tri_list->tris = NULL;
	last_tri_list->tri_count = 0;
	last_tri_list->offset = tri_lists_offset - 2;

	octree->root_node_count = root_node_count;
	octree->root_nodes = malloc(octree->root_node_count * sizeof(*octree->root_nodes));
	for (uint32_t i = 0; i < octree->root_node_count; i++)
	{
		if (!kcl_node_from_raw(&octree->root_nodes[i], &root_nodes[i],
			branches_offset, tri_lists, tri_list_count))
		{
			goto cleanup;
		}
	}

	octree->branch_count = branch_count;
	octree->branches = malloc(octree->branch_count * sizeof(*octree->branches));

	for (uint32_t i = 0; i < octree->branch_count; i++)
	{
		kcl_node_t* nodes = octree->branches[i].nodes;
		kcl_raw_node_t* raw_nodes = branches[i].nodes;

		for (int j = 0; j < 8; j++)
		{
			if (!kcl_node_from_raw(&nodes[j], &raw_nodes[j],
				branches_offset, tri_lists, tri_list_count))
			{
				goto cleanup;
			}
		}
	}

	octree->tri_list_count = tri_list_count;
	octree->tri_lists = malloc(octree->tri_list_count * sizeof(*octree->tri_lists));
	for (uint32_t i = 0; i < octree->tri_list_count; i++)
	{
		kcl_tri_list_t* tri_list = &octree->tri_lists[i];
		kcl_raw_tri_list_t* raw_tri_list = &tri_lists[i];

		tri_list->tris = raw_tri_list->tris;
		tri_list->tri_count = raw_tri_list->tri_count;
	}

	ret = 1;

cleanup:
	free(root_nodes);
	free(branches);
	free(tri_lists);

	return ret;
}

kcl_tri_list_t* kcl_octree_find(kcl_octree_t* octree, kcl_header_t* header, vec3_t* pos)
{
	uint32_t x = (uint32_t)(pos->x - header->origin.x);
	if ((x & header->x_mask) != 0)
		return NULL;

	uint32_t y = (uint32_t)(pos->y - header->origin.y);
	if ((y & header->y_mask) != 0)
		return NULL;

	uint32_t z = (uint32_t)(pos->z - header->origin.z);
	if ((z & header->z_mask) != 0)
		return NULL;

	uint32_t shift = header->shift;
	uint32_t node_idx = (z >> shift) << header->z_shift;
	node_idx |= (y >> shift) << header->y_shift;
	node_idx |= x >> shift;

	kcl_node_t* node = &octree->root_nodes[node_idx];
	for (;;)
	{
		if (node->type == kcl_node_leaf)
		{
			return &octree->tri_lists[node->idx];
		}
		else if (node->type == kcl_node_branch)
		{
			kcl_branch_t* branch = &octree->branches[node->idx];
			shift -= 1;
			node_idx = (z >> shift & 1) << 2 | (y >> shift & 1) << 1 | (x >> shift & 1);
			node = &branch->nodes[node_idx];
		}
	}
}

void kcl_init(kcl_t* kcl)
{
	memset(&kcl->header, 0, sizeof(kcl->header));
	kcl->tris = NULL;
	kcl->vertices = NULL;
	kcl->tri_count = 0;
	kcl->vertex_count = 0;
	kcl_octree_init(&kcl->octree);
}

void kcl_free(kcl_t* kcl)
{
	kcl_octree_free(&kcl->octree);
	free(kcl->tris);
	free(kcl->vertices);
	kcl->tris = NULL;
	kcl->vertices = NULL;
}

int kcl_parse(kcl_t* kcl, bin_t* bin)
{
	const uint32_t kcl_tri_size = 0x10;
	int ret = 0;

	bswapstream_t stream;
	bswapstream_init(&stream, bin->buffer);

	kcl_header_t* header = &kcl->header;

	header->pos_data_offset      = bswapstream_read_uint32(&stream);
	header->nrm_data_offset      = bswapstream_read_uint32(&stream);
	header->tri_data_offset      = bswapstream_read_uint32(&stream) + kcl_tri_size;
	header->octree_data_offset   = bswapstream_read_uint32(&stream);
	header->thickness            = bswapstream_read_float(&stream);
	header->origin               = bswapstream_read_vec3(&stream);
	header->x_mask               = bswapstream_read_uint32(&stream);
	header->y_mask               = bswapstream_read_uint32(&stream);
	header->z_mask               = bswapstream_read_uint32(&stream);
	header->shift                = bswapstream_read_uint32(&stream);
	header->y_shift              = bswapstream_read_uint32(&stream);
	header->z_shift              = bswapstream_read_uint32(&stream);
	header->sphere_radius		 = bswapstream_read_float(&stream);

	uint32_t pos_size = header->nrm_data_offset - header->pos_data_offset;
	uint32_t nrm_size = header->tri_data_offset - header->nrm_data_offset;
	uint32_t tri_size = header->octree_data_offset - header->tri_data_offset;
	uint32_t octree_size = (uint32_t)bin->size - header->octree_data_offset;

	if (pos_size % sizeof(vec3_t)
		|| nrm_size % sizeof(vec3_t)
		|| tri_size % kcl_tri_size)
	{
		printf("Error: unexpected KCL section size\n");
		return ret;
	}

	uint32_t pos_count = pos_size / sizeof(vec3_t);
	uint32_t nrm_count = nrm_size / sizeof(vec3_t);
	kcl->tri_count     = tri_size / sizeof(kcl_tri_size) >> 2;

	sizeof(kcl_tri_t);

	vec3_t* positions  = malloc(pos_size);
	vec3_t* normals    = malloc(nrm_size);
	kcl->tris          = malloc(sizeof(*kcl->tris) * kcl->tri_count);
	kcl->vertices      = malloc(sizeof(kcl_tri_vertex_t) * 3 * kcl->tri_count);

	bswapstream_seek(&stream, header->pos_data_offset);
	for (uint32_t i = 0; i < pos_count; i++)
		positions[i] = bswapstream_read_vec3(&stream);

	bswapstream_seek(&stream, header->nrm_data_offset);
	for (uint32_t i = 0; i < nrm_count; i++)
		normals[i]   = bswapstream_read_vec3(&stream);

	bswapstream_seek(&stream, header->tri_data_offset);
	for (uint32_t i = 0; i < kcl->tri_count; i++)
	{
		kcl_tri_t* tri    = &kcl->tris[i];
		uint16_t position_index, normal_index,
				 ca_normal_index, ab_normal_index, bc_normal_index;

		tri->height	      = bswapstream_read_float(&stream);
		position_index	  = bswapstream_read_uint16(&stream);
		normal_index      = bswapstream_read_uint16(&stream);
		ca_normal_index   = bswapstream_read_uint16(&stream);
		ab_normal_index   = bswapstream_read_uint16(&stream);
		bc_normal_index   = bswapstream_read_uint16(&stream);
		tri->attribute    = bswapstream_read_uint16(&stream);

		if (position_index >= pos_count
			|| normal_index >= nrm_count
			|| ca_normal_index >= nrm_count
			|| ab_normal_index >= nrm_count
			|| bc_normal_index >= nrm_count)
		{
			printf("Error, bad triangle at index %u in KCL\n", i);
			goto cleanup;
		}

		tri->position	  = positions[position_index];
		tri->normal	      = normals[normal_index];
		tri->ca_normal	  = normals[ca_normal_index];
		tri->ab_normal	  = normals[ab_normal_index];
		tri->bc_normal	  = normals[bc_normal_index];

		vec3_t cross_a, cross_b;
		vec3_cross(&tri->ca_normal, &tri->normal, &cross_a);
		vec3_cross(&tri->ab_normal, &tri->normal, &cross_b);

		if (KCL_ATTRIBUTE_TYPE_BIT(tri->attribute) & KCL_TYPE_SOLID_SURFACE)
		{
			kcl_tri_vertex_t* vertices = &kcl->vertices[(kcl->vertex_count++) * 3];
			vertices[0].position = tri->position;
			vec3_muladd(&tri->position, tri->height / vec3_dot(&cross_b, &tri->bc_normal), &cross_b, &vertices[1].position);
			vec3_muladd(&tri->position, tri->height / vec3_dot(&cross_a, &tri->bc_normal), &cross_a, &vertices[2].position);
			vec3_t color = kcl_flag_colors[KCL_ATTRIBUTE_TYPE(tri->attribute)];
			for (int j = 0; j < 3; j++)
			{
				vertices[j].normal = tri->normal;
				vertices[j].color = color;
			}
		}
	}

	bswapstream_seek(&stream, header->octree_data_offset);
	if (!kcl_octree_parse(&kcl->octree, &stream, octree_size))
		goto cleanup;

	ret = 1;

cleanup:
	if (ret != 1)
		kcl_octree_free(&kcl->octree);
	free(positions);
	free(normals);

	return ret;
}

void kcl_collision_hitbox(kcl_t* kcl, hitbox_t* hitbox, collision_t* collision)
{
	collision_init(collision);

	kcl_tri_list_t* tri_list = kcl_octree_find(&kcl->octree, &kcl->header, &hitbox->pos);
	if (tri_list)
	{
		float thickness = kcl->header.thickness;
		for (uint32_t i = 0; i < tri_list->tri_count; i++)
		{
			kcl_tri_t* tri = &kcl->tris[tri_list->tris[i]];
			collision_tri_t collision_tri;
			if (kcl_tri_collision_hitbox(tri, hitbox, thickness, &collision_tri))
			{
				collision_add_tri(collision, &collision_tri);
			}
		}
	}
}

void kcl_write_obj(kcl_t* kcl, const char* name)
{
	FILE* obj = fopen(name, "w");
	if (!obj)
	{
		printf("Failed to open %s for writing\n", name);
		return;
	}

	for (uint32_t i = 0; i < kcl->tri_count; i++)
	{
		for (int j = 0; j < 3; j++)
			fprintf(obj, "v %.6f %.6f %.6f\n", XYZ(kcl->vertices[i * 3].position));
	}

	int index = 1;
	for (uint32_t i = 0; i < kcl->tri_count; i++)
	{
		fprintf(obj, "f %d %d %d\n",
			index++, index++, index++);
	}

	fclose(obj);
}

parser_t kcl_parser =
{
	kcl_init,
	kcl_free,
	kcl_parse,
};