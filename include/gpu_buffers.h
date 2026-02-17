#ifndef GPU_BUFFERS_H
# define GPU_BUFFERS_H

# include <stdint.h>

typedef struct s_gpu_buffers
{
	uint32_t	vertex_ssbo;
	uint32_t	triangle_ssbo;
	uint32_t	material_ssbo;

	uint32_t	accumulation_texture;

	uint32_t	compute_prigram;
	uint32_t	fullscreen_program;
}	t_gpu_buffers;

#endif
