#pragma once

typedef enum
{
	shader_basic,
	shader_basic_vcolor,

	shader_type_last,
} shader_type;

typedef struct graphics_t graphics_t;

typedef struct shader_source_t
{
	shader_type  type;
	const char*  vertex;
	const char*  fragment;
	const char** uniforms;
	size_t       uniform_size;
	size_t		 uniforms_offset;
	const char** attributes;
	size_t       attribute_size;
	size_t		 attributes_offset;
	size_t		 size;
} shader_source_t;

typedef struct shader_t
{
	unsigned int  vertex;
	unsigned int  fragment;
	unsigned int  program;
	int*		  uniforms;
	int*		  attributes;
} shader_t;

int shader_compile(shader_t* shader, shader_source_t* source);
int shader_compile_all(shader_t** shaders);