#ifndef CYCLES_H
# define CYCLES_H

#include "../glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

# define TRIANGLE_VERTS 1
# define TRIANGLE_NORMS 2
# define TEXCOORDS_SSBOS 3
# define MESHES_SSBOS 4
# define BVH_SSBOS 5
# define TLAS_SSBOS 6
# define MATERIALS_SSBOS 7
# define LIGHT_SSBOS 8
# define IMAGES_SSBOS 9
# define PIXELS_SSBOS 10

# define NO_TONEMAP 0
# define AGX_TONEMAP 1

typedef struct s_cycles
{
	GLFWwindow		*win;
	GLuint			compute_program;
	GLuint			fullscreen_program;
	GLuint			tex;
	GLuint			vao;
	unsigned int	tonemap;
}	t_cycles;

t_cycles	init_cycles(void);
void		parse_cycles_args(t_cycles *cycles, char **args, int argv);

#endif
