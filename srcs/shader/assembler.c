#include "shader.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static char	*join_sources(const char *first_path, va_list args)
{
	va_list		args_copy;
	const char	*path;
	char		*src;
	char		*joined;
	const char	*version = "#version 430 core\n";
	size_t		total_len;
	size_t		version_len;

	va_copy(args_copy, args);
	version_len = strlen(version);
	total_len = version_len;
	path = first_path;
	while (path)
	{
		src = read_file(path);
		if (!src)
		{
			va_end(args_copy);
			return (NULL);
		}
		total_len += strlen(src);
		free(src);
		path = va_arg(args_copy, const char *);
	}
	va_end(args_copy);

	joined = malloc(total_len + 1);
	if (!joined)
		return (NULL);
	strcpy(joined, version);
	path = first_path;
	while (path)
	{
		src = read_file(path);
		if (!src)
		{
			free(joined);
			return (NULL);
		}
		strcat(joined, src);
		free(src);
		path = va_arg(args, const char *);
	}
	return (joined);
}

uint32_t	shader_create_compute_asm(const char *first_path, ...)
{
	va_list		args;
	char		*joined;
	uint32_t	cs;
	uint32_t	program;
	int			success;
	char		log[1024];

	va_start(args, first_path);
	joined = join_sources(first_path, args);
	va_end(args);
	if (!joined)
		return (0);
	cs = compile(GL_COMPUTE_SHADER, joined);
	free(joined);
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
