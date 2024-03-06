#include "../common.h"

#include "shader.h"
#include "shader_basic_vcolor.h"

#define SHADER_NAME shader_basic_vcolor_t

static const char* source_uniforms[] =
{
	"MVP", "vLightDir"
};
static const char* source_attributes[] =
{
	"vPos", "vNorm", "vCol",
};

shader_source_t shader_basic_vcolor_source =
{
    shader_basic_vcolor,

    "uniform mat4 MVP;\n"
    "attribute vec3 vPos;\n"
    "attribute vec3 vNorm;\n"
    "attribute vec3 vCol;\n"
    "varying vec3 fNormal;\n"
    "varying vec3 fColor;\n"
    "void main()\n"
    "{\n"
    "    fNormal = vNorm;\n"
    "    fColor = vCol;\n"
    "    gl_Position = MVP * vec4(vPos, 1);\n"
    "}\n",

    "varying vec3 fNormal;\n"
    "varying vec3 fColor;\n"
    "uniform vec3 vLightDir;\n"
    "void main()\n"
    "{\n"
    "    vec3 diffuse = max(dot(fNormal, vLightDir), 0.0) * fColor;\n"
    "    vec3 ambient = vec3(0.6, 0.6, 0.6) * fColor;\n"
    "    vec3 color = ambient + diffuse * ambient;\n"
    "    gl_FragColor = vec4(color, 1);\n"
    "}\n",

    source_uniforms,
    ARRAY_LEN(source_uniforms),
    offsetof(SHADER_NAME, uniforms),
    source_attributes,
    ARRAY_LEN(source_attributes),
    offsetof(SHADER_NAME, attributes),
    sizeof(SHADER_NAME),
};

STATIC_ASSERT(ARRAY_LEN(source_uniforms) == STRUCT_ARRAY_LEN(SHADER_NAME, uniforms));
STATIC_ASSERT(ARRAY_LEN(source_attributes) == STRUCT_ARRAY_LEN(SHADER_NAME, attributes));