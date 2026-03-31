#include "mesh.h"
#include "rt_math.h"
#include <stdlib.h>
#include <math.h>

static void	compute_cylinder_faces(t_vec4 *p, float y0, float y1, float radius, float j, float slices)
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
	p[0] = vec4_create(radius * cos_theta[0], y0, radius * sin_theta[0], 0.0f);
	p[1] = vec4_create(radius * cos_theta[0], y1, radius * sin_theta[0], 0.0f);
	p[2] = vec4_create(radius * cos_theta[1], y1, radius * sin_theta[1], 0.0f);
	p[3] = vec4_create(radius * cos_theta[1], y0, radius * sin_theta[1], 0.0f);
}

static void	compute_cylinder_normals(t_vec4 *n, float j, float slices)
{
	float	theta0;
	float	theta1;

	theta0 = 2.0f * M_PI * (j / slices);
	theta1 = 2.0f * M_PI * ((j + 1) / slices);
	n[0] = vec4_create(cosf(theta0), 0.0f, sinf(theta0), 0.0f);
	n[1] = vec4_create(cosf(theta0), 0.0f, sinf(theta0), 0.0f);
	n[2] = vec4_create(cosf(theta1), 0.0f, sinf(theta1), 0.0f);
	n[3] = vec4_create(cosf(theta1), 0.0f, sinf(theta1), 0.0f);
}

static void	compute_cylinder_uvs(t_vec4 *uv, int slices, int stacks, int i, int j)
{
	uv[0] = vec4_create((float)j / slices, (float)i / stacks, 0.0f, 0.0f);
	uv[1] = vec4_create((float)j / slices, (float)(i + 1) / stacks, 0.0f, 0.0f);
	uv[2] = vec4_create((float)(j + 1) / slices, (float)(i + 1) / stacks, 0.0f, 0.0f);
	uv[3] = vec4_create((float)(j + 1) / slices, (float)i / stacks, 0.0f, 0.0f);
}

static void	compute_cylinder_caps(t_triangle *triangles, int *index, int slices, float radius, float height)
{
	t_vec4	cap_center[2];
	t_vec4	cap_normal[2];
	t_vec4	p[4];
	t_vec4	uv[4];
	float	factor;
	float	theta[2];
	float	cos_theta[2];
	float	sin_theta[2];

	factor = height / 2.0f;
	cap_center[0] = vec4_create(0.0f, factor, 0.0f, 0.0f);
	cap_center[1] = vec4_create(0.0f, -factor, 0.0f, 0.0f);
	cap_normal[0] = vec4_create(0.0f, 1.0f, 0.0f, 0.0f);
	cap_normal[1] = vec4_create(0.0f, -1.0f, 0.0f, 0.0f);
	for (int j = 0; j < slices; j++)
	{
		theta[0] = 2.0f * M_PI * ((float)j / slices);
		theta[1] = 2.0f * M_PI * ((float)(j + 1) / slices);
		cos_theta[0] = cosf(theta[0]);
		sin_theta[0] = sinf(theta[0]);
		cos_theta[1] = cosf(theta[1]);
		sin_theta[1] = sinf(theta[1]);
		p[0] = vec4_create(radius * cos_theta[0], factor, radius * sin_theta[0], 0.0f);
		p[1] = vec4_create(radius * cos_theta[1], factor, radius * sin_theta[1], 0.0f);
		p[2] = vec4_create(radius * cos_theta[0], -factor, radius * sin_theta[0], 0.0f);
		p[3] = vec4_create(radius * cos_theta[1], -factor, radius * sin_theta[1], 0.0f);
		uv[0] = vec4_create((float)j / slices, 0.0f, 0.0f, 0.0f);
		uv[1] = vec4_create((float)(j + 1) / slices, 0.0f, 0.0f, 0.0f);
		uv[2] = vec4_create((float)j / slices, 1.0f, 0.0f, 0.0f);
		uv[3] = vec4_create((float)(j + 1) / slices, 1.0f, 0.0f, 0.0f);
		triangles[(*index)++] = (t_triangle){ cap_center[0], p[0], p[1], cap_normal[0], cap_normal[0], cap_normal[0], uv[0], uv[0], uv[1] };
		triangles[(*index)++] = (t_triangle){ cap_center[1], p[3], p[2], cap_normal[1], cap_normal[1], cap_normal[1], uv[3], uv[2], uv[2] };
	}
}

t_mesh	generate_cylinder(int stacks, int slices, float radius, float height)
{
	t_mesh	mesh;
	t_vec4	p[4];
	t_vec4	n[4];
	t_vec4	uv[4];

	mesh.triangle_count = stacks * slices * 2 + slices * 2;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = vec4_create(0.0f, 0.0f, 0.0f, sqrtf(radius * radius + height * height));
	mesh.scale = vec4_create(1.0f, 1.0f, 1.0f, 0.0f);
	mesh.smooth = 1;

	int index = 0;
	for (int i = 0; i < stacks; i++)
	{
		float y0 = height * ((float)i / stacks) - height / 2.0f;
		float y1 = height * ((float)(i + 1) / stacks) - height / 2.0f;
		for (int j = 0; j < slices; j++)
		{
			compute_cylinder_faces(p, y0, y1, radius, j, slices);
			compute_cylinder_uvs(uv, slices, stacks, i, j);
			compute_cylinder_normals(n, j, slices);
			mesh.triangles[index++] = (t_triangle){ p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1], uv[2] };
			mesh.triangles[index++] = (t_triangle){ p[0], p[2], p[3], n[0], n[2], n[3], uv[0], uv[2], uv[3] };
		}
	}
	compute_cylinder_caps(mesh.triangles, &index, slices, radius, height);
	return (mesh);
}