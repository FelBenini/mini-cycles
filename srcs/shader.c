#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include "../glad/include/glad/glad.h"

static char	*read_file(const char *path)
{
	FILE	*file;
	char	*buffer;
	long	size;

	file = fopen(path, "rb");
	if (!file)
	{
		printf("Failed to open shader: %s\n", path);
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);
	buffer = malloc(size + 1);
	if (!buffer)
	{
		fclose(file);
		return (NULL);
	}
	fread(buffer, 1, size, file);
	buffer[size] = 0;
	fclose(file);
	return (buffer);
}

static uint32_t compile(GLenum type, const char *source)
{
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, log);
        printf("Shader compile error:\n%s\n", log);
        exit(1);
    }

    return shader;
}

uint32_t shader_create_compute(const char *path)
{
    char *src = read_file(path);
    uint32_t cs = compile(GL_COMPUTE_SHADER, src);
    free(src);

    uint32_t program = glCreateProgram();
    glAttachShader(program, cs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, NULL, log);
        printf("Compute link error:\n%s\n", log);
        exit(1);
    }

    glDeleteShader(cs);
    return program;
}

uint32_t shader_create_graphics(const char *vert_path, const char *frag_path)
{
    char *vsrc = read_file(vert_path);
    char *fsrc = read_file(frag_path);

    uint32_t vs = compile(GL_VERTEX_SHADER, vsrc);
    uint32_t fs = compile(GL_FRAGMENT_SHADER, fsrc);

    free(vsrc);
    free(fsrc);

    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, NULL, log);
        printf("Program link error:\n%s\n", log);
        exit(1);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
