#include "scene.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static float	compute_bounding_radius(t_mesh *mesh)
{
	float		max_r2;
	t_triangle	*tri;
	float		r2;

	max_r2 = 0.0f;
	for (uint32_t i = 0; i < mesh->triangle_count; i++)
	{
		tri = &mesh->triangles[i];
		t_vec4 verts[3] = {tri->v0, tri->v1, tri->v2};
		for (int v = 0; v < 3; v++)
		{
			r2 = verts[v].x * verts[v].x + verts[v].y * verts[v].y + verts[v].z
				* verts[v].z;
			if (r2 > max_r2)
				max_r2 = r2;
		}
	}
	return (sqrtf(max_r2));
}

static void	rebuild_flat_triangle_array(t_scene *scene)
{
	uint32_t	offset;
	t_mesh		*mesh;

	free(scene->triangles);
	scene->triangle_count = 0;
	for (uint32_t m = 0; m < scene->mesh_count; m++)
		scene->triangle_count += scene->meshes[m].triangle_count;
	scene->triangles = malloc(sizeof(t_triangle) * scene->triangle_count);
	if (!scene->triangles)
	{
		fprintf(stderr, "scene: failed to allocate triangle array\n");
		return ;
	}
	offset = 0;
	for (uint32_t m = 0; m < scene->mesh_count; m++)
	{
		mesh = &scene->meshes[m];
		memcpy(scene->triangles + offset, mesh->triangles, sizeof(t_triangle)
			* mesh->triangle_count);
		scene->descriptors[m].tri_offset = offset;
		scene->descriptors[m].tri_count = mesh->triangle_count;
		offset += mesh->triangle_count;
	}
}

t_scene	scene_create(uint32_t initial_capacity)
{
	t_scene	scene = {0};

	scene.mesh_capacity = initial_capacity ? initial_capacity : 8;
	scene.meshes = malloc(sizeof(t_mesh) * scene.mesh_capacity);
	scene.descriptors = malloc(sizeof(t_mesh_descriptor) * scene.mesh_capacity);
	glGenBuffers(1, &scene.ssbo_triangles);
	glGenBuffers(1, &scene.ssbo_meshes);
	return (scene);
}

void	scene_destroy(t_scene *scene)
{
	for (uint32_t m = 0; m < scene->mesh_count; m++)
		free(scene->meshes[m].triangles);
	free(scene->meshes);
	free(scene->descriptors);
	free(scene->triangles);
	glDeleteBuffers(1, &scene->ssbo_triangles);
	glDeleteBuffers(1, &scene->ssbo_meshes);
	memset(scene, 0, sizeof(*scene));
}

uint32_t	scene_add_mesh(t_scene *scene, t_mesh mesh)
{
	uint32_t	index;

	if (scene->mesh_count == scene->mesh_capacity)
	{
		scene->mesh_capacity *= 2;
		scene->meshes = realloc(scene->meshes, sizeof(t_mesh)
				* scene->mesh_capacity);
		scene->descriptors = realloc(scene->descriptors,
				sizeof(t_mesh_descriptor) * scene->mesh_capacity);
	}
	index = scene->mesh_count++;
	scene->meshes[index] = mesh;
	scene->descriptors[index] = (t_mesh_descriptor){.position = mesh.position,
		.tri_offset = 0, .tri_count = mesh.triangle_count, .pad = {0, 0}};
	scene->descriptors[index].position.w = compute_bounding_radius(&mesh);
	scene->gpu_dirty = 1;
	scene->desc_dirty = 1;
	return (index);
}

void	scene_upload_triangles(t_scene *scene)
{
	rebuild_flat_triangle_array(scene);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_triangles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(t_triangle)
		* scene->triangle_count, scene->triangles, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene->ssbo_triangles);
	scene->gpu_dirty = 0;
	scene_upload_descriptors(scene);
}

void	scene_upload_descriptors(t_scene *scene)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_meshes);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(t_mesh_descriptor)
		* scene->mesh_count, scene->descriptors, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, scene->ssbo_meshes);
	scene->desc_dirty = 0;
}

void	scene_move_mesh(t_scene *scene, uint32_t index, t_vec4 position)
{
	if (index >= scene->mesh_count)
		return ;
	position.w = scene->descriptors[index].position.w;
	scene->meshes[index].position = position;
	scene->descriptors[index].position = position;
	scene->desc_dirty = 1;
}
