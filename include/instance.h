#ifndef INSTANCE_H
# define INSTANCE_H

# include <stdlib.h>
# include <string.h>
# include "material.h"
# include "mesh.h"
# include "rt_math.h"

typedef struct s_instance
{
	t_mesh		*mesh;
	t_material	material;

	t_vec4		position;
	t_vec4		scale;
}	t_instance;

#endif
