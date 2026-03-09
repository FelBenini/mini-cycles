#include <stdlib.h>
#include <string.h>
#include "scene.h"

int	scene_add_light(t_scene *scene, t_light light)
{
	t_light		*new_lights;
	uint32_t	new_capacity;

	if (!scene)
		return (0);
	if (scene->light_count >= scene->light_capacity)
	{
		if (scene->light_capacity == 0)
			new_capacity = 4;
		else
			new_capacity = scene->light_capacity * 2;
		new_lights = realloc(scene->lights, sizeof(t_light) * new_capacity);
		if (!new_lights)
			return (0);

		scene->lights = new_lights;
		scene->light_capacity = new_capacity;
	}
	scene->lights[scene->light_count] = light;
	scene->light_count++;
	return (1);
}

void	scene_upload_lights(t_scene *scene)
{
	if (!scene)
		return;

	if (scene->ssbo_lights == 0)
		glGenBuffers(1, &scene->ssbo_lights);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->ssbo_lights);
	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		sizeof(t_light) * scene->light_count,
		scene->lights,
		GL_DYNAMIC_DRAW
	);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, scene->ssbo_lights);
}
