#include "scene.h"
#include "cycles.h"

void	scene_upload_descriptors(t_scene *scene)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_meshes);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(t_mesh_descriptor) * scene->mesh_count,
			scene->descriptors, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MESHES_SSBOS, scene->ssbo_meshes);
	scene->desc_dirty = 0;
}
