#include "mesh.h"
#include "rt_math.h"
#include <stdlib.h>
#include <math.h>

static void	compute_cone_faces(t_vec4 *p, float y0, float y1, float r0, float r1, float j, float slices)
{
	float	theta0;
	float	theta1;
	float	cos_theta[2];
	float	sin_theta[2];

	theta0 = 2.0f * M_PI * (j / slices);
	theta1 = 2.0f * M_PI * ((j + 1) / slices);
	cos_theta[0] = cosf(theta0);
	sin_theta[0] = sinf(theta0);
	cos_theta[1] = cosf(theta1);
	sin_theta[1] = sinf(theta1);
	p[0] = vec4_create(r0 * cos_theta[0], y0, r0 * sin_theta[0], 0.0f);
	p[1] = vec4_create(r1 * cos_theta[0], y1, r1 * sin_theta[0], 0.0f);
	p[2] = vec4_create(r1 * cos_theta[1], y1, r1 * sin_theta[1], 0.0f);
	p[3] = vec4_create(r0 * cos_theta[1], y0, r0 * sin_theta[1], 0.0f);
}

static void	compute_cone_normals(t_vec4 *n, float radius, float height, float j, float slices)
{
	float	theta0;
	float	theta1;
	float	slope;
	float	len;
	float	cos_theta[2];
	float	sin_theta[2];

	slope = radius / height;
	len = sqrtf(1.0f + slope * slope);
	theta0 = 2.0f * M_PI * (j / slices);
	theta1 = 2.0f * M_PI * ((j + 1) / slices);
	cos_theta[0] = cosf(theta0);
	sin_theta[0] = sinf(theta0);
	cos_theta[1] = cosf(theta1);
	sin_theta[1] = sinf(theta1);
	n[0] = vec4_create(cos_theta[0] / len, slope / len, sin_theta[0] / len, 0.0f);
	n[1] = vec4_create(cos_theta[0] / len, slope / len, sin_theta[0] / len, 0.0f);
	n[2] = vec4_create(cos_theta[1] / len, slope / len, sin_theta[1] / len, 0.0f);
	n[3] = vec4_create(cos_theta[1] / len, slope / len, sin_theta[1] / len, 0.0f);
}

static void	compute_cone_uvs(t_vec4 *uv, int slices, int stacks, int i, int j)
{
	uv[0] = vec4_create((float)j / slices, (float)i / stacks, 0.0f, 0.0f);
	uv[1] = vec4_create((float)j / slices, (float)(i + 1) / stacks, 0.0f, 0.0f);
	uv[2] = vec4_create((float)(j + 1) / slices, (float)(i + 1) / stacks, 0.0f, 0.0f);
	uv[3] = vec4_create((float)(j + 1) / slices, (float)i / stacks, 0.0f, 0.0f);
}

static void	compute_cone_base(t_triangle *triangles, int *index, int slices, float radius, float height)
{
	t_vec4	base_normal;
	t_vec4	center;
	t_vec4	p[2];
	t_vec4	uv[2];
	float	factor;
	float	theta[2];
	float	cos_theta[2];
	float	sin_theta[2];

	factor = height / 2.0f;
	base_normal = vec4_create(0.0f, -1.0f, 0.0f, 0.0f);
	center = vec4_create(0.0f, -factor, 0.0f, 0.0f);
	for (int j = 0; j < slices; j++)
	{
		theta[0] = 2.0f * M_PI * ((float)j / slices);
		theta[1] = 2.0f * M_PI * ((float)(j + 1) / slices);
		cos_theta[0] = cosf(theta[0]);
		sin_theta[0] = sinf(theta[0]);
		cos_theta[1] = cosf(theta[1]);
		sin_theta[1] = sinf(theta[1]);
		p[0] = vec4_create(radius * cos_theta[0], -factor, radius * sin_theta[0], 0.0f);
		p[1] = vec4_create(radius * cos_theta[1], -factor, radius * sin_theta[1], 0.0f);
		uv[0] = vec4_create((float)j / slices, 0.0f, 0.0f, 0.0f);
		uv[1] = vec4_create((float)(j + 1) / slices, 0.0f, 0.0f, 0.0f);
		triangles[(*index)++] = (t_triangle){ center, p[1], p[0], base_normal, base_normal, base_normal, uv[0], uv[1], uv[1] };
	}
}

t_mesh	generate_cone(int stacks, int slices, float radius, float height)
{
	t_mesh	mesh;
	t_vec4	p[4];
	t_vec4	n[4];
	t_vec4	uv[4];

	mesh.triangle_count = stacks * slices * 2 + slices;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = vec4_create(0.0f, 0.0f, 0.0f, sqrtf(radius * radius + height * height));
	mesh.scale = vec4_create(1.0f, 1.0f, 1.0f, 0.0f);
	mesh.smooth = 1;

	int index = 0;
	for (int i = 0; i < stacks; i++)
	{
		float t0 = (float)i / stacks;
		float t1 = (float)(i + 1) / stacks;
		float r0 = radius * (1.0f - t0);
		float r1 = radius * (1.0f - t1);
		float y0 = height * t0 - height / 2.0f;
		float y1 = height * t1 - height / 2.0f;
		for (int j = 0; j < slices; j++)
		{
			compute_cone_faces(p, y0, y1, r0, r1, j, slices);
			compute_cone_uvs(uv, slices, stacks, i, j);
			compute_cone_normals(n, radius, height, j, slices);
			mesh.triangles[index++] = (t_triangle){ p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1], uv[2] };
			mesh.triangles[index++] = (t_triangle){ p[0], p[2], p[3], n[0], n[2], n[3], uv[0], uv[2], uv[3] };
		}
	}
	compute_cone_base(mesh.triangles, &index, slices, radius, height);
	return (mesh);
}
