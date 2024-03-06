typedef struct shader_basic_vcolor_t
{
	shader_t base;
	union
	{
		struct
		{
			int uniform_mvp;
			int uniform_light_dir;
		};

		int uniforms[2];
	};
	union
	{
		struct
		{
			int attribute_pos;
			int attribute_norm;
			int attribute_color;
		};

		int attributes[3];
	};
} shader_basic_vcolor_t;

extern shader_source_t shader_basic_vcolor_source;