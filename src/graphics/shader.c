#include "../common.h"

#include "GL/glew.h"
#include "graphics.h"

#include "shader.h"

#include "shader_basic.h"
#include "shader_basic_vcolor.h"

int shader_compile(shader_t* shader, shader_source_t* source)
{
    GLint status;
    GLchar error_log[1024];

    shader->vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader->vertex, 1, &source->vertex, NULL);
    glCompileShader(shader->vertex);
    glGetShaderiv(shader->vertex, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetShaderInfoLog(shader->vertex, sizeof(error_log), NULL, error_log);
        printf("** Failed to compile vertex shadder:\n%s", error_log);
        return 0;
    }

    shader->fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader->fragment, 1, &source->fragment, NULL);
    glCompileShader(shader->fragment);
    glGetShaderiv(shader->fragment, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetShaderInfoLog(shader->fragment, sizeof(error_log), NULL, error_log);
        printf("** Failed to compile fragment shadder:\n%s\n", error_log);
        return 0;
    }

    shader->program = glCreateProgram();
    glAttachShader(shader->program, shader->vertex);
    glAttachShader(shader->program, shader->fragment);

    int* attributes = (int*)((uint8_t*)shader + source->attributes_offset);
    for (size_t i = 0; i < source->attribute_size; i++)
    {
        const char* attribute_name = source->attributes[i];
        glBindAttribLocation(shader->program, i, attribute_name);
        attributes[i] = (int)i;
    }

    glLinkProgram(shader->program);
    glGetShaderiv(shader->program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetProgramInfoLog(shader->program, sizeof(error_log), NULL, error_log);
        printf("** Failed to link shader:\n%s\n", error_log);
        return 0;
    }

    int* uniforms = (int*)((uint8_t*)shader + source->uniforms_offset);
    for (size_t i = 0; i < source->uniform_size; i++)
    {
        const char* uniform_name = source->uniforms[i];
        int uniform = glGetUniformLocation(shader->program, uniform_name);
        if (uniform == -1)
        {
            printf("Failed to find uniform %s\n", uniform_name);
            return 0;
        }
        uniforms[i] = uniform;
    }

    return 1;
}

shader_source_t* shader_sources[shader_type_last] =
{
    &shader_basic_source,
    &shader_basic_vcolor_source,
};

STATIC_ASSERT(ARRAY_LEN(shader_sources) == shader_type_last);

int shader_compile_all(shader_t** shaders)
{
    *shaders = malloc(shader_type_last * sizeof(shader_t*));
    for (size_t i = 0; i < shader_type_last; i++)
    {
        shader_source_t* source = shader_sources[i];
        shaders[i] = malloc(source->size);
        if (!shader_compile(shaders[i], source))
            return 0;
    }

    return 1;
}