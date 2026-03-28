#include <stdlib.h>
#include <stdio.h>
#include "cycles.h"
#include "scene.h"

void	scene_build_emissive_list(t_scene *scene)
{
	uint32_t	i;
	uint32_t	count;

	free(scene->emissive_mesh_indices);
	scene->emissive_mesh_indices = NULL;
	scene->emissive_mesh_count = 0;
	if (scene->mesh_count == 0)
		return ;
	scene->emissive_mesh_indices = malloc(sizeof(uint32_t) * scene->mesh_count);
	if (!scene->emissive_mesh_indices)
		return ;
	count = 0;
	i = 0;
	while (i < scene->mesh_count)
	{
		t_material *mat = &scene->materials[scene->descriptors[i].material];
		if (mat->emission.x > 0.0f || mat->emission.y > 0.0f
			|| mat->emission.z > 0.0f)
		{
			scene->emissive_mesh_indices[count] = i;
			count++;
		}
		i++;
	}
	scene->emissive_mesh_count = count;
}

void	scene_upload_emissive_meshes(t_scene *scene)
{
	if (!scene)
		return ;
	if (scene->ssbo_emissive_meshes == 0)
		glGenBuffers(1, &scene->ssbo_emissive_meshes);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_emissive_meshes);
	if (scene->emissive_mesh_count > 0)
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			sizeof(uint32_t) * scene->emissive_mesh_count,
			scene->emissive_mesh_indices, GL_DYNAMIC_DRAW);
	else
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t),
			NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, EMISSIVE_SSBOS,
		scene->ssbo_emissive_meshes);
}
