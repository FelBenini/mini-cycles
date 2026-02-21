#include "../glad/include/glad/glad.h"
#include "init.h"
#include "mesh.h"
#include "rt_math.h"
#include "scene.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 1920
#define HEIGHT 1080

int	main(void)
{
	GLFWwindow	*window;
	float		time;
	GLuint		vao;
	GLuint		compute_program;
	GLuint		fullscreen_program;
	t_scene		scene;
	t_mesh		sphere1;
	t_mesh		sphere2;
	uint32_t	idx1;
	uint32_t	idx2;
	GLuint		tex;
	GLint		loc_resolution;
	GLint		loc_mesh_count;
	GLint		loc_accumulation_tex_fs;

	window = init_cycles();
	time = 0.0f;
	(void)time;
	glViewport(0, 0, WIDTH, HEIGHT);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	compute_program = shader_create_compute("shaders/pathtrace.comp.glsl");
	fullscreen_program = shader_create_graphics("shaders/fullscreen.vert.glsl",
			"shaders/fullscreen.frag.glsl");
	scene = scene_create(8);
	sphere1 = generate_uv_sphere(32, 32, 1.0f);
	sphere1.smooth = 0;
	sphere1.position = (t_vec4){-1.0f, 0.0f, -3.0f, 0.0f};
	idx1 = scene_add_mesh(&scene, sphere1);
	sphere2 = generate_uv_sphere(32, 32, 1.0f);
	sphere2.position = (t_vec4){-1.0f, 0.0f, -3.0f, 0.0f};
	idx2 = scene_add_mesh(&scene, sphere2);
	(void)idx1;

	scene_upload_triangles(&scene);
	scene_upload_bvh_nodes(&scene);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA,
		GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	loc_resolution = glGetUniformLocation(compute_program, "u_resolution");
	loc_mesh_count = glGetUniformLocation(compute_program, "u_mesh_count");
	loc_accumulation_tex_fs = glGetUniformLocation(fullscreen_program, "u_accumulation_tex");
	while (!glfwWindowShouldClose(window))
	{
		time += 0.016f;
		scene_move_mesh(&scene, idx1, (t_vec4){sinf(time) * 2.0f, cosf(time), -3.0f,
			0.0f});
		scene_move_mesh(&scene, idx2, (t_vec4){sinf(time) * -2.0f, -cosf(time), -3.0f,
			0.0f});
		if (scene.desc_dirty)
			scene_upload_descriptors(&scene);
		glUseProgram(compute_program);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.ssbo_triangles);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, scene.ssbo_meshes);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.ssbo_bvh_nodes);
		glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glUniform2f(loc_resolution, (float)WIDTH, (float)HEIGHT);
		glUniform1ui(loc_mesh_count, scene.mesh_count);
		glDispatchCompute((WIDTH + 7) / 8, (HEIGHT + 7) / 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(fullscreen_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(loc_accumulation_tex_fs, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	scene_destroy(&scene);
	glfwTerminate();
	return (0);
}
