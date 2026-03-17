#include "rt_math.h"
#include "scene.h"
#include <stdio.h>
#include <string.h>

void	process_light(t_scene *scene, char *line)
{
	float	intensity;
	t_vec3	color = {0};
	t_vec3	direction = {0};
	t_vec3	position = {0};
	char	type[10];
	t_light	light = {0};
	float	cos_inner;
	float	cos_outer;

	cos_inner = 1.0f;
	cos_outer = 1.0f;
	if (sscanf(line, "L %f %f,%f,%f %s %f,%f,%f %f,%f,%f %f %f",
				&intensity,
				&color.x, &color.y, &color.z,
				type,
				&direction.x, &direction.y, &direction.z,
				&position.x, &position.y, &position.z,
				&cos_inner, &cos_outer) < 4)
	{
		printf("Error: Invalid light format.\n");
		return ;
	}
	color.x /= 255;
	color.y /= 255;
	color.z /= 255;
	light.color = color;
	light.type = 1;
	if (strcmp(type, "SUN") == 0)
		light.type = 0;
	if (strcmp(type, "POINT") == 0)
		light.type = 1;
	if (strcmp(type, "SPOT") == 0)
		light.type = 2;
	light.intensity = intensity;
	light.direction = direction;
	light.position = position;
	light.cos_inner = cos_inner;
	light.cos_outer = cos_outer;
	scene_add_light(scene, light);
}
