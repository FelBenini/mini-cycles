#include "mesh.h"
#include <stdlib.h>
#include <math.h>

t_mesh	generate_cylinder(int stacks, int slices, float radius, float height)
{
	t_mesh	mesh;

	mesh.triangle_count = stacks * slices * 2 + slices * 2;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = (t_vec4){0.0f, 0.0f, 0.0f, sqrtf(radius * radius + height * height)};
	mesh.scale = (t_vec4){1.0f, 1.0f, 1.0f, 0.0f};
	mesh.smooth = 1;
	int index = 0;
	for (int i = 0; i < stacks; i++)
	{
		float y0 = height * ((float)i / stacks) - height / 2.0f;
		float y1 = height * ((float)(i + 1) / stacks) - height / 2.0f;
		for (int j = 0; j < slices; j++)
		{
			float theta0 = 2.0f * M_PI * ((float)j / slices);
			float theta1 = 2.0f * M_PI * ((float)(j + 1) / slices);
			t_vec4 p0 = { radius * cosf(theta0), y0, radius * sinf(theta0), 0.0f };
			t_vec4 p1 = { radius * cosf(theta0), y1, radius * sinf(theta0), 0.0f };
			t_vec4 p2 = { radius * cosf(theta1), y1, radius * sinf(theta1), 0.0f };
			t_vec4 p3 = { radius * cosf(theta1), y0, radius * sinf(theta1), 0.0f };
			t_vec4 n0 = { cosf(theta0), 0.0f, sinf(theta0), 0.0f };
			t_vec4 n1 = { cosf(theta0), 0.0f, sinf(theta0), 0.0f };
			t_vec4 n2 = { cosf(theta1), 0.0f, sinf(theta1), 0.0f };
			t_vec4 n3 = { cosf(theta1), 0.0f, sinf(theta1), 0.0f };
			mesh.triangles[index++] = (t_triangle){ p0, p1, p2, n0, n1, n2 };
			mesh.triangles[index++] = (t_triangle){ p0, p2, p3, n0, n2, n3 };
		}
	}
	t_vec4 top_normal    = { 0.0f,  1.0f, 0.0f, 0.0f };
	t_vec4 bottom_normal = { 0.0f, -1.0f, 0.0f, 0.0f };
	t_vec4 top_center    = { 0.0f,  height / 2.0f, 0.0f, 0.0f };
	t_vec4 bot_center    = { 0.0f, -height / 2.0f, 0.0f, 0.0f };
	for (int j = 0; j < slices; j++)
	{
		float theta0 = 2.0f * M_PI * ((float)j / slices);
		float theta1 = 2.0f * M_PI * ((float)(j + 1) / slices);
		t_vec4 t0 = { radius * cosf(theta0),  height / 2.0f, radius * sinf(theta0), 0.0f };
		t_vec4 t1 = { radius * cosf(theta1),  height / 2.0f, radius * sinf(theta1), 0.0f };
		t_vec4 b0 = { radius * cosf(theta0), -height / 2.0f, radius * sinf(theta0), 0.0f };
		t_vec4 b1 = { radius * cosf(theta1), -height / 2.0f, radius * sinf(theta1), 0.0f };
		mesh.triangles[index++] = (t_triangle){ top_center, t0, t1, top_normal,    top_normal,    top_normal    };
		mesh.triangles[index++] = (t_triangle){ bot_center, b1, b0, bottom_normal, bottom_normal, bottom_normal };
	}
	return (mesh);
}
