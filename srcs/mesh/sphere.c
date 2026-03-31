#include <math.h>
#include <stdlib.h>
#include "rt_math.h"
#include "triangle.h"
#include <math.h>
#include <stdlib.h>
#include "mesh.h"

static void	compute_faces(t_vec4 *p,
							float phi0,
							float phi1,
							float radius,
							float slices,
							float j)
{
	float	theta0;
	float	theta1;
	float	cosf_theta[2];
	float	sinf_theta[2];
	float	cosf_phi[2];
	float	sinf_phi[2];

	theta0 = 2.0f * M_PI * (j / slices);
	theta1 = 2.0f * M_PI * ((j + 1) / slices);
	cosf_theta[0] = cosf(theta0);
	sinf_theta[0] = sinf(theta0);
	cosf_theta[1] = cosf(theta1);
	sinf_theta[1] = sinf(theta1);
	cosf_phi[0] = cosf(phi0);
	cosf_phi[1] = cosf(phi1);
	sinf_phi[0] = sinf(phi0);
	sinf_phi[1] = sinf(phi1);
	p[0] = vec4_create(
		radius * sinf_phi[0] * cosf_theta[0],
		radius * cosf_phi[0],
		radius * sinf_phi[0] * sinf_theta[0],
		0.0f
	);
	p[1] = vec4_create(
		radius * sinf_phi[1] * cosf_theta[0],
		radius * cosf_phi[1],
		radius * sinf_phi[1] * sinf_theta[0],
		0.0f
	);
	p[2] = vec4_create(
		radius * sinf_phi[1] * cosf_theta[1],
		radius * cosf_phi[1],
		radius * sinf_phi[1] * sinf_theta[1],
		0.0f
	);
	p[3] = vec4_create(
		radius * sinf_phi[0] * cosf_theta[1],
		radius * cosf_phi[0],
		radius * sinf_phi[0] * sinf_theta[1],
		0.0f
	);
}

static void	compute_normals(t_vec4 *n, t_vec4 *p, float radius)
{
	n[0] = vec4_create(p[0].x / radius, p[0].y / radius, p[0].z / radius, 0.0f);
	n[1] = vec4_create(p[1].x / radius, p[1].y / radius, p[1].z / radius, 0.0f);
	n[2] = vec4_create(p[2].x / radius, p[2].y / radius, p[2].z / radius, 0.0f);
	n[3] = vec4_create(p[3].x / radius, p[3].y / radius, p[3].z / radius, 0.0f);
}

static void	compute_uvs(t_vec4 *uv, int slices, int stacks, int i, int j)
{
	float	top;
	float	bottom;
	float	left;
	float	right;

	top = (float)i / stacks;
	bottom = (float)(i + 1) / stacks;
	left = (float)j / slices;
	right = (float)(j + 1) / slices;

	uv[0] = vec4_create(left, top, 0.0f, 0.0f);
	uv[1] = vec4_create(left, bottom, 0.0f, 0.0f);
	uv[2] = vec4_create(right, bottom, 0.0f, 0.0f);
	uv[3] = vec4_create(right, top, 0.0f, 0.0f);
}

t_mesh	generate_uv_sphere(int stacks, int slices, float radius)
{
	t_mesh	mesh;
	t_vec4	uv[4];
	t_vec4	p[4];
	t_vec4	n[4];

	mesh.triangle_count = stacks * slices * 2;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = (t_vec4){0.0f, 0.0f, 0.0f, radius};
	mesh.scale = (t_vec4){1.0f, 1.0f, 1.0f, 0.0f};
	mesh.smooth = 1;

	int index = 0;

	for (int i = 0; i < stacks; i++)
	{
		float phi0 = M_PI * ((float)i / stacks);
		float phi1 = M_PI * ((float)(i + 1) / stacks);
		for (int j = 0; j < slices; j++)
		{
			compute_faces(p, phi0, phi1, radius, slices, j);
			compute_uvs(uv, slices, stacks, i, j);
			compute_normals(n, p, radius);
			mesh.triangles[index++] = (t_triangle){ p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1], uv[2] };
			mesh.triangles[index++] = (t_triangle){ p[0], p[2], p[3], n[0], n[2], n[3], uv[0], uv[2], uv[3] };
		}
	}
	return (mesh);
}
