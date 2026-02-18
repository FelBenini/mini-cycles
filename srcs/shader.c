#include "../glad/include/glad/glad.h"
#include "shader.h"
#include <stdio.h>
#include <stdlib.h>

static char	*read_file(const char *path)
{
	FILE	*file;
	char	*buffer;
	long	size;
	long	bytes_read;

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
	bytes_read = fread(buffer, 1, size, file);
	if (bytes_read != size)
	{
		fprintf(stderr, "Error reading file\n");
		fclose(file);
		free(buffer);
		return (NULL);
	}
	buffer[size] = 0;
	fclose(file);
	return (buffer);
}

static uint32_t	compile(GLenum type, const char *source)
{
	uint32_t	shader;
	int			success;
	char		log[1024];

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 1024, NULL, log);
		printf("Shader compile error:\n%s\n", log);
		exit(1);
	}
	return (shader);
}

uint32_t	shader_create_compute(const char *path)
{
	char		*src;
	uint32_t	cs;
	uint32_t	program;
	int			success;
	char		log[1024];

	src = read_file(path);
	cs = compile(GL_COMPUTE_SHADER, src);
	free(src);
	program = glCreateProgram();
	glAttachShader(program, cs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 1024, NULL, log);
		printf("Compute link error:\n%s\n", log);
		exit(1);
	}
	glDeleteShader(cs);
	return (program);
}

uint32_t	shader_create_graphics(const char *vert_path, const char *frag_path)
{
	char		*vsrc;
	char		*fsrc;
	uint32_t	vs;
	uint32_t	fs;
	uint32_t	program;
	int			success;
	char		log[1024];

	vsrc = read_file(vert_path);
	fsrc = read_file(frag_path);
	vs = compile(GL_VERTEX_SHADER, vsrc);
	fs = compile(GL_FRAGMENT_SHADER, fsrc);
	free(vsrc);
	free(fsrc);
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 1024, NULL, log);
		printf("Program link error:\n%s\n", log);
		exit(1);
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
	return (program);
}
