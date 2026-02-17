#ifndef TRIANGLE_H
# define TRIANGLE_H
# include <stdint.h>

typedef struct s_triangle
{
	uint32_t	v0;
	uint32_t	v1;
	uint32_t	v2;
	uint32_t	material_index;
}	t_triangle;

#endif
