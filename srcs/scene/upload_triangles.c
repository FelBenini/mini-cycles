#include "bvh.h"
#include "mesh.h"
#include "scene.h"
#include "triangle.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static uint32_t	count_triangles_in_meshes(t_scene *scene)
{
	uint32_t	i;
	uint32_t	res;

	i = 0;
	res = 0;
	while (i < scene->mesh_count)
	{
		res += scene->meshes[i].triangle_count;
		i++;
	}
	return (res);
}

static int	rebuild_flat_triangle_array(t_scene *scene)
{
	uint32_t	offset;
	uint32_t	i;
	t_mesh		*curr_mesh;
	t_bvh		*bvh;
	
	free(scene->triangles);
	scene->triangle_count = count_triangles_in_meshes(scene);
	scene->triangles = malloc(sizeof(t_triangle) * scene->triangle_count);
	if (!scene->triangles)
		return (fprintf(stderr, "scene: failed to allocate triangle array\n"), 0);
	offset = 0;
	i = 0;
	while (i < scene->mesh_count)
	{
		curr_mesh = &scene->meshes[i];
		bvh = &scene->bvhs[i];
		if (bvh->indices && bvh->index_count > 0)
		{
			for (uint32_t j = 0; j < bvh->index_count; j++)
				scene->triangles[offset + j] = curr_mesh->triangles[bvh->indices[j]];
		}
		else
			memcpy(scene->triangles + offset, curr_mesh->triangles, sizeof(t_triangle) * curr_mesh->triangle_count);
		scene->descriptors[i].tri_offset = offset;
		scene->descriptors[i].tri_count = curr_mesh->triangle_count;
		scene->descriptors[i].smooth = curr_mesh->smooth;
		offset += curr_mesh->triangle_count;
		i++;
	}
	return (1);
}

void	scene_upload_triangles(t_scene *scene)
{
	rebuild_flat_triangle_array(scene);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_triangles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(t_triangle) * scene->triangle_count,
			scene->triangles, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene->ssbo_triangles);
	scene->gpu_dirty = 0;
	scene_upload_descriptors(scene);
	scene_upload_bvh_nodes(scene);
}
