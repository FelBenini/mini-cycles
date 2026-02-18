#include <stdlib.h>
#include "init.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

GLFWwindow	*init_cycles(void)
{
	GLFWwindow	*win;

	if (!glfwInit())
		exit(1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    win = glfwCreateWindow(WIDTH, HEIGHT, "GPU Pathtracer Skeleton", NULL, NULL);
    glfwMakeContextCurrent(win);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to load GLAD\n");
        exit(1);
    }
	return (win);
}
