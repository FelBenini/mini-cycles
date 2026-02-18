#include <stdio.h>
#include <stdlib.h>
#include "init.h"
#include "mesh.h"
#include <stdint.h>
#include "../glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "rt_math.h"
#include <math.h>
#include "shader.h"

#define WIDTH 800
#define HEIGHT 600

int main()
{
	GLFWwindow *window = init_cycles();
	float time = 0.0f;

	glViewport(0, 0, WIDTH, HEIGHT);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint compute_program =
		shader_create_compute("shaders/pathtrace.comp.glsl");

	GLuint fullscreen_program =
		shader_create_graphics(
			"shaders/fullscreen.vert.glsl",
			"shaders/fullscreen.frag.glsl");

	t_mesh sphere = generate_uv_sphere(12, 12, 1.0f);
	sphere.position = (t_vec4){0.0f, 0.0f, -3.0f, 0.0f};
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
				 sizeof(t_triangle) * sphere.triangle_count,
				 sphere.triangles,
				 GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);

	free(sphere.triangles);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
				 WIDTH, HEIGHT, 0,
				 GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glUseProgram(compute_program);
	GLint loc_resolution =
		glGetUniformLocation(compute_program, "u_resolution");
	GLint loc_tri_count =
		glGetUniformLocation(compute_program, "u_triangle_count");
	GLint loc_mesh_pos =
		glGetUniformLocation(compute_program, "u_mesh_position");

	while (!glfwWindowShouldClose(window))
	{
		time += 0.016f;

		sphere.position.x = sinf(time) * 2.0f;

		glUseProgram(compute_program);

		glBindImageTexture(0, tex, 0, GL_FALSE, 0,
						   GL_WRITE_ONLY, GL_RGBA32F);

		glUniform2f(loc_resolution, WIDTH, HEIGHT);
		glUniform1ui(loc_tri_count, sphere.triangle_count);
		glUniform3f(loc_mesh_pos,
					sphere.position.x,
					sphere.position.y,
					sphere.position.z);

		glDispatchCompute((WIDTH + 7)/8,
						  (HEIGHT + 7)/8,
						  1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(fullscreen_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);

		glUniform1i(
			glGetUniformLocation(fullscreen_program,
								 "u_accumulation_tex"),
			0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

