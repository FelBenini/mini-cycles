#include "mesh.h"
#include <stdio.h>
#include <stdlib.h>

t_mesh	load_mesh_from_obj(const char *filepath, float radius)
{
	t_mesh	mesh;
	FILE	*file;
	char	line[256];
	int		vert_count;
	int		norm_count;
	int		uv_count;
	int		face_count;
	t_vec4	*verts;
	t_vec4	*norms;
	t_vec4	*uvs;
	int		index;

	file = fopen(filepath, "r");
	if (!file)
	{
		mesh.triangle_count = 0;
		mesh.triangles = NULL;
		return (mesh);
	}

	// First pass: count vertices, normals, uvs, and total triangles after fan-triangulation
	vert_count = 0;
	norm_count = 0;
	uv_count = 0;
	face_count = 0;
	while (fgets(line, sizeof(line), file))
	{
		if (line[0] == 'v' && line[1] == ' ')
			vert_count++;
		else if (line[0] == 'v' && line[1] == 'n')
			norm_count++;
		else if (line[0] == 'v' && line[1] == 't')
			uv_count++;
		else if (line[0] == 'f' && line[1] == ' ')
		{
			int		verts_in_face = 0;
			char	*ptr = line + 2;
			char	*prev;
			while (*ptr)
			{
				while (*ptr == ' ' || *ptr == '\t') ptr++;
				if (!*ptr || *ptr == '\n' || *ptr == '\r')
					break;
				prev = ptr;
				while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
					ptr++;
				if (ptr == prev)
					break; // no progress, bail
				verts_in_face++;
			}
			if (verts_in_face >= 3)
				face_count += verts_in_face - 2;
		}
	}

	verts = malloc(sizeof(t_vec4) * vert_count);
	norms = malloc(sizeof(t_vec4) * norm_count);
	uvs = malloc(sizeof(t_vec4) * uv_count);

	mesh.triangle_count = face_count;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = (t_vec4){0.0f, 0.0f, 0.0f, radius};
	mesh.scale = (t_vec4){1.0f, 1.0f, 1.0f, 0.0f};
	mesh.smooth = 1;

	// Second pass: parse data
	rewind(file);
	int vi = 0, ni = 0, ui = 0;
	index = 0;
	while (fgets(line, sizeof(line), file))
	{
		if (line[0] == 'v' && line[1] == ' ')
		{
			float x, y, z;
			sscanf(line + 2, "%f %f %f", &x, &y, &z);
			verts[vi++] = (t_vec4){x, y, z, 0.0f};
		}
		else if (line[0] == 'v' && line[1] == 'n')
		{
			float x, y, z;
			sscanf(line + 3, "%f %f %f", &x, &y, &z);
			norms[ni++] = (t_vec4){x, y, z, 0.0f};
		}
		else if (line[0] == 'v' && line[1] == 't')
		{
			float u, v;
			sscanf(line + 3, "%f %f", &u, &v);
			uvs[ui++] = (t_vec4){u, v, 0.0f, 0.0f};
		}
		else if (line[0] == 'f' && line[1] == ' ')
		{
			int		face_v[64];
			int		face_vn[64];
			int		face_vt[64];
			int		face_vert_count = 0;
			char	*ptr = line + 2;
			while (*ptr && face_vert_count < 64)
			{
				while (*ptr == ' ' || *ptr == '\t') ptr++;
				if (!*ptr || *ptr == '\n' || *ptr == '\r') break;

				int v = 0, vn = 0, vt = 0;
				int matched = 0;
				if (sscanf(ptr, "%d/%d/%d", &v, &vt, &vn) == 3)
					matched = 1;
				else if (sscanf(ptr, "%d//%d", &v, &vn) == 2)
					matched = 1;
				else if (sscanf(ptr, "%d/%d", &v, &vt) == 2)
					matched = 1;
				else if (sscanf(ptr, "%d", &v) == 1)
					matched = 1;
				if (!matched)
					break;
				face_v[face_vert_count] = v;
				face_vn[face_vert_count] = vn;
				face_vt[face_vert_count] = vt;
				face_vert_count++;
				while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
					ptr++;
			}

			// Fan-triangulate: anchor at face_v[0], emit (n-2) triangles
			for (int f = 1; f + 1 < face_vert_count; f++)
			{
				int idx[3] = { 0, f, f + 1 };
				t_vec4 p[3], n[3], uv[3];

				for (int k = 0; k < 3; k++)
				{
					p[k] = verts[face_v[idx[k]] - 1];
					p[k].x *= radius;
					p[k].y *= radius;
					p[k].z *= radius;
				}

				if (norm_count > 0 && face_vn[0] > 0)
				{
					for (int k = 0; k < 3; k++)
					{
						n[k] = norms[face_vn[idx[k]] - 1];
						n[k].x *= radius;
						n[k].y *= radius;
						n[k].z *= radius;
					}
				}
				else
				{
					t_vec4 e1 = {p[1].x-p[0].x, p[1].y-p[0].y, p[1].z-p[0].z, 0.0f};
					t_vec4 e2 = {p[2].x-p[0].x, p[2].y-p[0].y, p[2].z-p[0].z, 0.0f};
					t_vec4 fn = {
						e1.y*e2.z - e1.z*e2.y,
						e1.z*e2.x - e1.x*e2.z,
						e1.x*e2.y - e1.y*e2.x,
						0.0f
					};
					float len = sqrtf(fn.x*fn.x + fn.y*fn.y + fn.z*fn.z);
					fn.x /= len; fn.y /= len; fn.z /= len;
					n[0] = n[1] = n[2] = fn;
				}

				if (uv_count > 0 && face_vt[0] > 0)
				{
					for (int k = 0; k < 3; k++)
						uv[k] = uvs[face_vt[idx[k]] - 1];
				}
				else
				{
					uv[0] = uv[1] = uv[2] = (t_vec4){0.0f, 0.0f, 0.0f, 0.0f};
				}

				mesh.triangles[index++] = (t_triangle){p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1], uv[2]};
			}
		}
	}
	fclose(file);
	free(verts);
	free(norms);
	free(uvs);
	return (mesh);
}
