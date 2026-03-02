#include "mesh.h"
#include <stdlib.h>

t_mesh	generate_cube(float size)
{
	t_mesh	mesh;

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
		t_vec4 p0 = {faces[i][0][0], faces[i][0][1], faces[i][0][2], 0.0f};
		t_vec4 p1 = {faces[i][1][0], faces[i][1][1], faces[i][1][2], 0.0f};
		t_vec4 p2 = {faces[i][2][0], faces[i][2][1], faces[i][2][2], 0.0f};
		t_vec4 p3 = {faces[i][3][0], faces[i][3][1], faces[i][3][2], 0.0f};
		t_vec4 n  = {faces[i][4][0], faces[i][4][1], faces[i][4][2], 0.0f};
		mesh.triangles[index++] = (t_triangle){ p0, p1, p2, n, n, n };
		mesh.triangles[index++] = (t_triangle){ p0, p2, p3, n, n, n };
	}
	return (mesh);
}
