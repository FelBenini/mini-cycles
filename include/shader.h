#ifndef SHADER_H
# define SHADER_H

# include <stdint.h>
# include "../glad/include/glad/glad.h"

char		*read_file(const char *path);
uint32_t	compile(GLenum type, const char *source);
uint32_t	shader_create_compute(const char *path);
uint32_t	shader_create_graphics(const char *vert_path, const char *frag_path);
uint32_t	shader_create_compute_asm(const char *first_path, ...);

#endif
