#ifndef SCENE_H
# define SCENE_H

# include "material.h"
# include "rt_math.h"
# include "triangle.h"
# include <stdint.h>

typedef struct s_scene
{
	t_vertex	*vertices;
	uint32_t	vertex_count;

	t_triangle	*triangles;
	uint32_t	triangle_count;

	t_material	*materials;
	uint32_t	material_count;
}   t_scene;

#endif
