#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "shader.h"

#define WIDTH 800
#define HEIGHT 600

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "GPU Pathtracer Skeleton", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to load GLAD\n");
        return -1;
    }
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
    GLuint compute_program = shader_create_compute("shaders/pathtrace.comp.glsl");
	GLuint fullscreen_program =
    	shader_create_graphics(
        	"shaders/fullscreen.vert.glsl",
        	"shaders/fullscreen.frag.glsl"
    	);    GLuint accumulation_tex;
    glGenTextures(1, &accumulation_tex);
    glBindTexture(GL_TEXTURE_2D, accumulation_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint frame_index = 0;

    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(compute_program);

        glBindImageTexture(0, accumulation_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        glUniform1ui(glGetUniformLocation(compute_program, "u_frame_index"), frame_index);
        glUniform2f(glGetUniformLocation(compute_program, "u_resolution"), WIDTH, HEIGHT);

        glDispatchCompute((WIDTH + 7)/8, (HEIGHT + 7)/8, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(fullscreen_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumulation_tex);

        glUniform1i(glGetUniformLocation(fullscreen_program, "u_accumulation_tex"), 0);
        glUniform1ui(glGetUniformLocation(fullscreen_program, "u_frame_index"), frame_index);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_index++;
    }

    glfwTerminate();
    return 0;
}
