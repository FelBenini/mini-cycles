#include "rt_math.h"
#include "triangle.h"
#include <float.h>
#include <math.h>
#include <stdint.h>

// Axis - aligned bound box

// get The bounding box of a triangle
static void	get_triangle_bbox(t_triangle tri, t_vec4 *min_out, t_vec4 *max_out)
{
	float	min_x;
	float	min_y;
	float	min_z;
	float	max_x;
	float	max_y;
	float	max_z;

	min_x = fminf(tri.v0.x, fminf(tri.v1.x, tri.v2.x));
	min_y = fminf(tri.v0.y, fminf(tri.v1.y, tri.v2.y));
	min_z = fminf(tri.v0.z, fminf(tri.v1.z, tri.v2.z));
	max_x = fmaxf(tri.v0.x, fmaxf(tri.v1.x, tri.v2.x));
	max_y = fmaxf(tri.v0.y, fmaxf(tri.v1.y, tri.v2.y));
	max_z = fmaxf(tri.v0.z, fmaxf(tri.v1.z, tri.v2.z));
	min_out->x = min_x;
	min_out->y = min_y;
	min_out->z = min_z;
	max_out->x = max_x;
	max_out->y = max_y;
	max_out->z = max_z;
}

void	compute_bbox(t_triangle *triangles, uint32_t *indices, uint32_t start,
						uint32_t count, t_vec4 *out_min, t_vec4 *out_max)
{
	float		min_x = FLT_MAX, min_y = FLT_MAX, min_z = FLT_MAX;
	float		max_x = -FLT_MAX, max_y = -FLT_MAX, max_z = -FLT_MAX;
	t_vec4		tri_min;
	t_vec4		tri_max;
	uint32_t	i;

	if (count == 0)
	{
		out_min->x = out_min->y = out_min->z = 0.0f;
		out_max->x = out_max->y = out_max->z = 0.0f;
		out_min->w = out_max->w = 0.0f;
		return ;
	}
	i = start;
	// loop between the triangles and find min and max between them
	while (i < start + count)
	{
		get_triangle_bbox(triangles[indices[i]], &tri_min, &tri_max);
		min_x = fminf(min_x, tri_min.x);
    	min_y = fminf(min_y, tri_min.y);
    	min_z = fminf(min_z, tri_min.z);
		max_x = fmaxf(max_x, tri_max.x);
		max_y = fmaxf(max_y, tri_max.y);
		max_z = fmaxf(max_z, tri_max.z);
		i++;
	}
	out_min->x = min_x;
	out_min->y = min_y;
	out_min->z = min_z;
	out_max->x = max_x;
	out_max->y = max_y;
	out_max->z = max_z;
}
