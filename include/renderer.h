#ifndef RENDERER_H
# define RENDERER_H

# include "scene.h"
# include "gpu_buffers.h"
# include "camera.h"
# include <stdint.h>

typedef struct s_renderer
{
	uint32_t		width;
	uint32_t		height;

	uint32_t		frame_index;

	t_scene			scene;
	t_camera		camera;
	t_gpu_buffers	gpu;
}	t_renderer;

#endif
