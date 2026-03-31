#include "mesh.h"
#include "rt_math.h"
#include <stdlib.h>
#include <math.h>

static void	compute_faces(t_vec4 *p, float *cos_theta, float *sin_theta,
							float *cos_phi, float *sin_phi,
							float major_radius, float minor_radius)
{
	// Torus parametric formula:
	// x = (R + r*cos(phi)) * cos(theta)
	// y =  r * sin(phi)
	// z = (R + r*cos(phi)) * sin(theta)
	p[0] = vec4_create(
		(major_radius + minor_radius * cos_phi[0]) * cos_theta[0],
		minor_radius * sin_phi[0],
		(major_radius + minor_radius * cos_phi[0]) * sin_theta[0],
		0.0f
	);
	p[1] = vec4_create(
		(major_radius + minor_radius * cos_phi[0]) * cos_theta[1],
		minor_radius * sin_phi[0],
		(major_radius + minor_radius * cos_phi[0]) * sin_theta[1],
		0.0f
	);
	p[2] = vec4_create(
		(major_radius + minor_radius * cos_phi[1]) * cos_theta[1],
		minor_radius * sin_phi[1],
		(major_radius + minor_radius * cos_phi[1]) * sin_theta[1],
		0.0f
	);
	p[3] = vec4_create(
		(major_radius + minor_radius * cos_phi[1]) * cos_theta[0],
		minor_radius * sin_phi[1],
		(major_radius + minor_radius * cos_phi[1]) * sin_theta[0],
		0.0f
	);
}

static void	compute_normals(t_vec4 *n, t_vec4 *p, float *cos_theta, float *sin_theta, float major_radius, float minor_radius)
{
	// Normals point away from the tube center ring (not the origin)
	// Center of the tube at each point lies on the major circle
	t_vec4 c0 = { major_radius * cos_theta[0], 0.0f, major_radius * sin_theta[0], 0.0f };
	t_vec4 c1 = { major_radius * cos_theta[1], 0.0f, major_radius * sin_theta[1], 0.0f };
	n[0] = vec4_create((p[0].x - c0.x) / minor_radius, p[0].y / minor_radius, (p[0].z - c0.z) / minor_radius, 0.0f);
	n[1] = vec4_create((p[1].x - c1.x) / minor_radius, p[1].y / minor_radius, (p[1].z - c1.z) / minor_radius, 0.0f);
	n[2] = vec4_create((p[2].x - c1.x) / minor_radius, p[2].y / minor_radius, (p[2].z - c1.z) / minor_radius, 0.0f);
	n[3] = vec4_create((p[3].x - c0.x) / minor_radius, p[3].y / minor_radius, (p[3].z - c0.z) / minor_radius, 0.0f);
}

static void	compute_uvs(t_vec4 *uv, float i, float j, float stacks, float slices)
{
	uv[0] = vec4_create((float)j / slices, (float)i / stacks, 0.0f, 0.0f);
	uv[1] = vec4_create((float)j / slices, (float)(i + 1) / stacks, 0.0f, 0.0f);
	uv[2] = vec4_create((float)(j + 1) / slices, (float)(i + 1) / stacks, 0.0f, 0.0f);
	uv[3] = vec4_create((float)(j + 1) / slices, (float)i / stacks, 0.0f, 0.0f);
}

t_mesh	generate_torus(int stacks, int slices, float major_radius, float minor_radius)
{
	t_mesh	mesh;
	float	cos_phi[2];
	float	sin_phi[2];
	float	cos_theta[2];
	float	sin_theta[2];
	t_vec4	p[4];
	t_vec4	n[4];
	t_vec4	uv[4];

	mesh.triangle_count = stacks * slices * 2;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = vec4_create(0.0f, 0.0f, 0.0f, 0.0f);
	mesh.scale = vec4_create(1.0f, 1.0f, 1.0f, 0.0f);
	mesh.smooth = 1;
	int index = 0;
	for (int i = 0; i < stacks; i++)
	{
		float theta0 = 2.0f * M_PI * ((float)i / stacks);
		float theta1 = 2.0f * M_PI * ((float)(i + 1) / stacks);
		cos_theta[0] = cosf(theta0);
		cos_theta[1] = cosf(theta1);
		sin_theta[0] = sinf(theta0);
		sin_theta[1] = sinf(theta1);
		for (int j = 0; j < slices; j++)
		{
			float phi0 = 2.0f * M_PI * ((float)j / slices);
			float phi1 = 2.0f * M_PI * ((float)(j + 1) / slices);
			cos_phi[0] = cosf(phi0);
			cos_phi[1] = cosf(phi1);
			sin_phi[0] = sinf(phi0);
			sin_phi[1] = sinf(phi1);

			compute_faces(p, cos_theta, sin_theta, cos_phi, sin_phi, major_radius, minor_radius);
			compute_normals(n, p, cos_theta, sin_theta, major_radius, minor_radius);
			compute_uvs(uv, i, j, stacks, slices);
			mesh.triangles[index++] = (t_triangle){ p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1], uv[2] };
			mesh.triangles[index++] = (t_triangle){ p[0], p[2], p[3], n[0], n[2], n[3], uv[0], uv[2], uv[3] };
		}
	}
	return (mesh);
}
