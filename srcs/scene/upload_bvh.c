#include "bvh.h"
#include "scene.h"
#include <stdint.h>
#include <stdlib.h>

static uint32_t	get_total_nodes(t_scene *scene)
{
	uint32_t	res;
	uint32_t	i;

	res = 0;
	i = 0;
	while (i < scene->mesh_count)
	{
		res += scene->bvhs[i].node_count;
		i++;
	}
	return (res);
}

static t_bvh_node	*get_flattened_nodes(t_scene *scene, uint32_t total_nodes)
{
	t_bvh_node	*res;
	uint32_t	node_offset;
	uint32_t	i;
	t_bvh		*bvh;

	res = malloc(sizeof(t_bvh_node) * total_nodes);
	if (!res)
		return (NULL);
	node_offset = 0;
	i = 0;
	while (i < scene->mesh_count)
	{
		bvh = &scene->bvhs[i];
		scene->descriptors[i].bvh_root = node_offset;
		for (uint32_t j = 0; j < bvh->node_count; j++)
		{
			t_bvh_node node = bvh->nodes[j];
			if (node.tri_count == 0)
			{
				node.left_child += node_offset;
				node.right_child += node_offset;
			}
			res[node_offset + j] = node;
		}
		node_offset += bvh->node_count;
		i++;
	}
	return (res);
}

void	scene_upload_bvh_nodes(t_scene *scene)
{
	uint32_t	total_nodes;
	t_bvh_node	*flattened;
	
	total_nodes = get_total_nodes(scene);
	flattened = get_flattened_nodes(scene, total_nodes);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_bvh_nodes);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(t_bvh_node) * total_nodes,
			flattened, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene->ssbo_bvh_nodes);
	free(flattened);
	scene_upload_descriptors(scene);
	scene->bvh_dirty = 0;
}

void	scene_rebuild_tlas(t_scene *scene)
{
	if (scene->tlas.nodes)
		tlas_destroy(&scene->tlas);
	
	if (scene->mesh_count > 0)
		scene->tlas = tlas_build(scene->descriptors, scene->mesh_count);
	
	scene->tlas_dirty = 1;
}

void	scene_upload_tlas_nodes(t_scene *scene)
{
	if (scene->tlas_dirty)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_tlas_nodes);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 
					sizeof(t_tlas_node) * scene->tlas.node_count,
					scene->tlas.nodes, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, scene->ssbo_tlas_nodes);
		scene->tlas_dirty = 0;
	}
}
