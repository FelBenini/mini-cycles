#include "mesh.h"
#include <stdlib.h>
#include <math.h>

t_mesh generate_cone(int stacks, int slices, float radius, float height)
{
	t_mesh mesh;
	// side triangles + base triangles
	mesh.triangle_count = stacks * slices * 2 + slices;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = (t_vec4){0.0f, 0.0f, 0.0f, 0.0f};
	mesh.scale = (t_vec4){1.0f, 1.0f, 1.0f, 0.0f};
	mesh.smooth = 1;
	int index = 0;

	for (int i = 0; i < stacks; i++)
	{
		float t0 = (float)i / stacks;
		float t1 = (float)(i + 1) / stacks;
		float r0 = radius * (1.0f - t0);
		float r1 = radius * (1.0f - t1);
		float y0 = height * t0;
		float y1 = height * t1;

		for (int j = 0; j < slices; j++)
		{
			float theta0 = 2.0f * M_PI * ((float)j / slices);
			float theta1 = 2.0f * M_PI * ((float)(j + 1) / slices);

			t_vec4 p0 = { r0 * cosf(theta0), y0, r0 * sinf(theta0), 0.0f };
			t_vec4 p1 = { r1 * cosf(theta0), y1, r1 * sinf(theta0), 0.0f };
			t_vec4 p2 = { r1 * cosf(theta1), y1, r1 * sinf(theta1), 0.0f };
			t_vec4 p3 = { r0 * cosf(theta1), y0, r0 * sinf(theta1), 0.0f };

			// Cone surface normal: perpendicular to the slant
			// The slope factor accounts for the tilt of the surface
			float slope = radius / height;
			float len = sqrtf(1.0f + slope * slope);
			t_vec4 n0 = { cosf(theta0) / len, slope / len, sinf(theta0) / len, 0.0f };
			t_vec4 n1 = { cosf(theta0) / len, slope / len, sinf(theta0) / len, 0.0f };
			t_vec4 n2 = { cosf(theta1) / len, slope / len, sinf(theta1) / len, 0.0f };
			t_vec4 n3 = { cosf(theta1) / len, slope / len, sinf(theta1) / len, 0.0f };

			mesh.triangles[index++] = (t_triangle){ p0, p1, p2, n0, n1, n2 };
			mesh.triangles[index++] = (t_triangle){ p0, p2, p3, n0, n2, n3 };
		}
	}

	// Base cap (flat disk at y = 0, normal pointing down)
	t_vec4 base_normal = { 0.0f, -1.0f, 0.0f, 0.0f };
	t_vec4 center = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (int j = 0; j < slices; j++)
	{
		float theta0 = 2.0f * M_PI * ((float)j / slices);
		float theta1 = 2.0f * M_PI * ((float)(j + 1) / slices);

		t_vec4 p0 = { radius * cosf(theta0), 0.0f, radius * sinf(theta0), 0.0f };
		t_vec4 p1 = { radius * cosf(theta1), 0.0f, radius * sinf(theta1), 0.0f };

		mesh.triangles[index++] = (t_triangle){ center, p1, p0, base_normal, base_normal, base_normal };
	}

	// Compute bounding radius
	float bounding = sqrtf(radius * radius + height * height);
	mesh.position.w = bounding;

	return (mesh);
}
