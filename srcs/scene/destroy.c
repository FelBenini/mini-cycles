#include "scene.h"
#include <stdlib.h>
#include <string.h>

void	scene_destroy(t_scene *scene)
{
	for (uint32_t m = 0; m < scene->mesh_count; m++)
	{
		free(scene->meshes[m].triangles);
		bvh_destroy(&scene->bvhs[m]);
	}
	free(scene->meshes);
	free(scene->descriptors);
	free(scene->materials);
	free(scene->triangles);
	free(scene->bvhs);
	free(scene->emissive_mesh_indices);
	scene_destroy_images(scene);
	tlas_destroy(&scene->tlas);
	glDeleteBuffers(1, &scene->ssbo_triangles);
	glDeleteBuffers(1, &scene->ssbo_normals);
	glDeleteBuffers(1, &scene->ssbo_texcoords);
	glDeleteBuffers(1, &scene->ssbo_meshes);
	glDeleteBuffers(1, &scene->ssbo_bvh_nodes);
	glDeleteBuffers(1, &scene->ssbo_tlas_nodes);
	glDeleteBuffers(1, &scene->ssbo_materials);
	glDeleteBuffers(1, &scene->ssbo_emissive_meshes);
	memset(scene, 0, sizeof(*scene));
}
