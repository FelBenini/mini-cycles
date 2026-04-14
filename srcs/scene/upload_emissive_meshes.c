#include "scene.h"
#include "cycles.h"

void	scene_upload_emissive_meshes(t_scene *scene)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_emissive_meshes);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * scene->emissive_mesh_count,
			scene->emissive_mesh_indices, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, EMISSIVE_MESHES_SSBOS, scene->ssbo_emissive_meshes);
	scene->emissive_mesh_dirty = 0;
}