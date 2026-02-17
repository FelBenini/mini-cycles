#ifndef SHADER_H
# define SHADER_H

# include <stdint.h>

uint32_t	shader_create_compute(const char *path);
uint32_t	shader_create_graphics(const char *vert_path, const char *frag_path);

#endif
