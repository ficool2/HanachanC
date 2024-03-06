typedef struct shader_basic_t
{
	shader_t base;
	union
	{
		struct
		{
			int uniform_mvp;
			int uniform_color;
			int uniform_light_dir;
		};

		int uniforms[3];
	};
	union
	{
		struct
		{
			int attribute_pos;
			int attribute_norm;
		};

		int attributes[2];
	};
} shader_basic_t;

extern shader_source_t shader_basic_source;