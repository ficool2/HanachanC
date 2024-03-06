#include "../common.h"
#include "bsp.h"

void bsp_init(bsp_t* bsp)
{
	bsp->hitbox_count = 0;
	bsp->wheel_count = 0;
}

void bsp_free(bsp_t* bsp)
{
	bsp->hitbox_count = 0;
	bsp->wheel_count = 0;
}

int bsp_parse(bsp_t* bsp, bin_t* bin)
{
	bswapstream_t stream;
	bswapstream_init(&stream, bin->buffer);

	bsp->y_offset = bswapstream_read_float(&stream);

	bsp->hitbox_count = 0;
	for (int i = 0; i < bsp_max_hitboxes; i++)
	{
		uint16_t enable = bswapstream_read_uint16(&stream);
		if (!enable)
		{
			bswapstream_read_skip(&stream, sizeof(bsp_hitbox_t) - sizeof(enable));
			continue;
		}

		bsp_hitbox_t* hitbox	    = &bsp->hitboxes[bsp->hitbox_count++];
		hitbox->enable				= 1;
		hitbox->pad					= bswapstream_read_uint16(&stream);
		hitbox->sphere_center		= bswapstream_read_vec3(&stream);
		hitbox->sphere_radius		= bswapstream_read_float(&stream);
		hitbox->wall_only			= bswapstream_read_uint16(&stream);
		hitbox->tire_index			= bswapstream_read_uint16(&stream);
	}

	bsp->cuboids[0]					= bswapstream_read_vec3(&stream);
	bsp->cuboids[1]					= bswapstream_read_vec3(&stream);
	bsp->angular_velocity_boost		= bswapstream_read_float(&stream);
									  bswapstream_read_float(&stream);

	bsp->wheel_count = 0;
	for (int i = 0; i < bsp_max_wheels; i++)
	{
		bsp_wheel_t* wheel          = &bsp->wheels[bsp->wheel_count++];

									  bswapstream_read_uint16(&stream);
		wheel->enable				= 1;
		wheel->pad					= bswapstream_read_uint16(&stream);
		wheel->suspension_distance	= bswapstream_read_float(&stream);
		wheel->suspension_speed		= bswapstream_read_float(&stream);
		wheel->suspension_slack		= bswapstream_read_float(&stream);
		wheel->suspension_top		= bswapstream_read_vec3(&stream);
		wheel->rotation_x			= bswapstream_read_float(&stream);
		wheel->radius				= bswapstream_read_float(&stream);
		wheel->sphere_radius		= bswapstream_read_float(&stream);
		wheel->unknown				= bswapstream_read_uint32(&stream);
	}

	bsp->rumble_distance			= bswapstream_read_float(&stream);
	bsp->rumble_speed				= bswapstream_read_float(&stream);

	if (bsp->wheel_count < 2)
	{
		printf("Error: BSP has less than 2 wheels\n");
		return 0;
	}

	return 1;
}

parser_t bsp_parser =
{
	bsp_init,
	bsp_free,
	bsp_parse,
};