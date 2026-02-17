#ifndef HIT_RECORD_H
# define HIT_RECORD_H

# include "rt_math.h"
# include <stdint.h>

typedef struct s_hit
{
	float		t;
	t_vec3		position;
	t_vec3		normal;
	uint32_t	material_index;
	int			hit;
}	t_hit;

#endif
