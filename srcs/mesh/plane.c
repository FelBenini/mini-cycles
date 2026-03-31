#include "mesh.h"
#include "rt_math.h"
#include <stdlib.h>

t_mesh	generate_plane(float width, float depth)
{
	t_mesh	mesh;
	t_vec4	p[4];
	t_vec4	n;
	t_vec4	uv[4];
	float	hw;
	float	hd;

	mesh.triangle_count = 2;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = vec4_create(0.0f, 0.0f, 0.0f, 0.0f);
	mesh.scale = vec4_create(1.0f, 1.0f, 1.0f, 0.0f);
	mesh.smooth = 1;
	hw = width * 0.5f;
	hd = depth * 0.5f;
	p[0] = vec4_create(-hw, 0.0f, -hd, 0.0f);
	p[1] = vec4_create(hw, 0.0f, -hd, 0.0f);
	p[2] = vec4_create(hw, 0.0f,  hd, 0.0f);
	p[3] = vec4_create(-hw, 0.0f,  hd, 0.0f);
	uv[0] = vec4_create(0.0f, 0.0f, 0.0f, 0.0f);
	uv[1] = vec4_create(1.0f, 0.0f, 0.0f, 0.0f);
	uv[2] = vec4_create(1.0f, 1.0f, 0.0f, 0.0f);
	uv[3] = vec4_create(0.0f, 1.0f, 0.0f, 0.0f);
	n = vec4_create(0.0f, 1.0f, 0.0f, 0.0f);
	mesh.triangles[0] = (t_triangle){ p[0], p[2], p[1], n, n, n, uv[0], uv[2], uv[1] };
	mesh.triangles[1] = (t_triangle){ p[0], p[3], p[2], n, n, n, uv[0], uv[3], uv[2] };
	return (mesh);
}
