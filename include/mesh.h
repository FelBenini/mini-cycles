#ifndef MESH_H
# define MESH_H

# include <stdint.h>

typedef struct s_mesh {
	uint32_t	first_triangle;
	uint32_t	triangle_count;
	uint32_t	material_offset;
	uint32_t	pad;
}	t_mesh;

#endif
