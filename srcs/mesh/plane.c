#include "mesh.h"
#include <stdlib.h>

t_mesh	generate_plane(float width, float depth)
{
	t_mesh	mesh;

	mesh.triangle_count = 2;
	mesh.triangles = malloc(sizeof(t_triangle) * mesh.triangle_count);
	mesh.position = (t_vec4){0.0f, 0.0f, 0.0f, 0.0f};
	mesh.scale = (t_vec4){1.0f, 1.0f, 1.0f, 0.0f};
	mesh.smooth = 1;
	float hw = width * 0.5f;
	float hd = depth * 0.5f;
	t_vec4 p0 = { -hw, 0.0f, -hd, 0.0f };
	t_vec4 p1 = {  hw, 0.0f, -hd, 0.0f };
	t_vec4 p2 = {  hw, 0.0f,  hd, 0.0f };
	t_vec4 p3 = { -hw, 0.0f,  hd, 0.0f };
	t_vec4 n = { 0.0f, 1.0f, 0.0f, 0.0f };
	mesh.triangles[0] = (t_triangle){ p0, p2, p1, n, n, n };
	mesh.triangles[1] = (t_triangle){ p0, p3, p2, n, n, n };
	return (mesh);
}
