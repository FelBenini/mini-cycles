#include "mesh.h"
#include "rt_math.h"
#include <stdlib.h>

t_mesh	generate_cube(float size)
{
	t_mesh	mesh;
	t_vec4	p[4];
	t_vec4	uv[4];
	t_vec4	n;

	mesh.triangle_count = 12;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = (t_vec4){0.0f, 0.0f, 0.0f, size};
	mesh.scale = (t_vec4){1.0f, 1.0f, 1.0f, 0.0f};
	mesh.smooth = 0;
	float h = size / 2.0f;
	int index = 0;
	float faces[6][5][3] = {
		{{ h,-h,-h},{ h,-h, h},{ h, h, h},{ h, h,-h},{ 1, 0, 0}},
		{{-h,-h, h},{-h,-h,-h},{-h, h,-h},{-h, h, h},{-1, 0, 0}},
		{{-h, h,-h},{ h, h,-h},{ h, h, h},{-h, h, h},{ 0, 1, 0}},
		{{-h,-h, h},{ h,-h, h},{ h,-h,-h},{-h,-h,-h},{ 0,-1, 0}},
		{{-h,-h, h},{-h, h, h},{ h, h, h},{ h,-h, h},{ 0, 0, 1}},
		{{ h,-h,-h},{ h, h,-h},{-h, h,-h},{-h,-h,-h},{ 0, 0,-1}},
	};

	for (int i = 0; i < 6; i++)
	{
		p[0] = vec4_create(faces[i][0][0], faces[i][0][1], faces[i][0][2], 0.0f);
		p[1] = vec4_create(faces[i][1][0], faces[i][1][1], faces[i][1][2], 0.0f);
		p[2] = vec4_create(faces[i][2][0], faces[i][2][1], faces[i][2][2], 0.0f);
		p[3] = vec4_create(faces[i][3][0], faces[i][3][1], faces[i][3][2], 0.0f);
		n  = vec4_create(faces[i][4][0], faces[i][4][1], faces[i][4][2], 0.0f);
		uv[0] = vec4_create(0.0f, 0.0f, 0.0f, 0.0f);
		uv[1] = vec4_create(1.0f, 0.0f, 0.0f, 0.0f);
		uv[2] = vec4_create(1.0f, 1.0f, 0.0f, 0.0f);
		uv[3] = vec4_create(0.0f, 1.0f, 0.0f, 0.0f);
		mesh.triangles[index++] = (t_triangle){ p[0], p[1], p[2], n, n, n, uv[0], uv[1], uv[2] };
		mesh.triangles[index++] = (t_triangle){ p[0], p[2], p[3], n, n, n, uv[0], uv[2], uv[3] };
	}
	return (mesh);
}
