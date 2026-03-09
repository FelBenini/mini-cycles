#include <stdlib.h>
#include "shader.h"
#include "cycles.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080

static GLuint	gen_tex(void)
{
	GLuint	tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA,
		GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return (tex);
}

static GLuint	gen_vao(void)
{
	GLuint	vao;

	glViewport(0, 0, WIDTH, HEIGHT);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	return (vao);
}

t_cycles	init_cycles(void)
{
	t_cycles cycles;

	if (!glfwInit())
		exit(1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    cycles.win = glfwCreateWindow(WIDTH, HEIGHT, "miniCycles", NULL, NULL);
    glfwMakeContextCurrent(cycles.win);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to load GLAD\n");
        exit(1);
    }
	cycles.compute_program = shader_create_compute_asm("shaders/structs.comp.glsl",
																	"shaders/triangle.comp.glsl",
																	"shaders/mat_from_dir.comp.glsl",
																	"shaders/intersect_aabb.comp.glsl",
																	"shaders/blas_intersect.comp.glsl",
																	"shaders/scene_intersect.comp.glsl",
																	"shaders/seed.comp.glsl",
																	"shaders/sky.comp.glsl",
																	"shaders/pathtrace.comp.glsl",
																	NULL);
	cycles.fullscreen_program = shader_create_graphics("shaders/fullscreen.vert.glsl",
			"shaders/fullscreen.frag.glsl");
	cycles.tex = gen_tex();
	cycles.vao = gen_vao();
	return (cycles);
}
