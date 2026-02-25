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
	free(scene->triangles);
	free(scene->bvhs);
	glDeleteBuffers(1, &scene->ssbo_triangles);
	glDeleteBuffers(1, &scene->ssbo_meshes);
	glDeleteBuffers(1, &scene->ssbo_bvh_nodes);
	memset(scene, 0, sizeof(*scene));
}
