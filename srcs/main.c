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

void	create_balls(t_scene *scene)
{
	int	i;
	int	j;
	t_mesh	ball;

	i = 0;
	while (i < 20)
	{
		j = 0;
		while (j < 10)
		{
			ball = generate_uv_sphere(32, 32, 0.3f);
			ball.smooth = 0;
			ball.position = (t_vec4){(i * 0.8f) - 4.0f, (j * 0.8f) - 4.0f, -5.0f, 0.0f};
			scene_add_mesh(scene, ball);
			j++;
		}
		i++;
	}
}

int	main(void)
{
	GLFWwindow	*window;
	GLuint		vao;
	GLuint		compute_program;
	GLuint		fullscreen_program;
	t_scene		scene;
	GLuint		tex;
	GLint		loc_resolution;
	GLint		loc_mesh_count;
	GLint		loc_accumulation_tex_fs;

	double		last_time;
	double		current_time;
	int			frame_count;
	double		fps;
	char		title[256];

	window = init_cycles();
	last_time = glfwGetTime();
	frame_count = 0;
	fps = 0.0;

	glViewport(0, 0, WIDTH, HEIGHT);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	compute_program = shader_create_compute("shaders/pathtrace.comp.glsl");
	fullscreen_program = shader_create_graphics("shaders/fullscreen.vert.glsl",
			"shaders/fullscreen.frag.glsl");
	scene = scene_create(8);
	t_mesh  cone = generate_cone(32, 32, 1.0f, 3.0f);
	t_mesh  cone2 = generate_cone(32, 32, 1.0f, 3.0f);
  t_mesh  plane = generate_plane(10.0f, 10.0f);
	cone.position = (t_vec4){-2.0f, -1.0f, -3.0f, 0.0f};
	cone2.position = (t_vec4){2.0f, -1.0f, -3.0f, 0.0f};
	plane.position = (t_vec4){0.0f, -1.0f, 0.0f, 0.0f};
	scene_add_mesh(&scene, cone);
	scene_add_mesh(&scene, cone2);
	scene_add_mesh(&scene, plane);
	scene_upload_triangles(&scene);
	scene_upload_bvh_nodes(&scene);
	scene_rebuild_tlas(&scene);
	scene_upload_tlas_nodes(&scene);
	printf("%d\n", scene.triangle_count);
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
	float time = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		time += 0.6f;
		if (scene.desc_dirty)
			scene_upload_descriptors(&scene);
		if (scene.tlas_dirty)
		{
			scene_rebuild_tlas(&scene);
			scene_upload_tlas_nodes(&scene);
		}
		glUseProgram(compute_program);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.ssbo_triangles);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, scene.ssbo_meshes);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.ssbo_bvh_nodes);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, scene.ssbo_tlas_nodes);
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
		current_time = glfwGetTime();
		frame_count++;

		if (current_time - last_time >= 1.0)
		{
			fps = frame_count / (current_time - last_time);
			double ms = 1000.0 / fps;

			snprintf(title, sizeof(title),
				"PathTracer | FPS: %.2f | %.2f ms",
				fps, ms);

			glfwSetWindowTitle(window, title);

			frame_count = 0;
			last_time = current_time;
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	scene_destroy(&scene);
	glfwTerminate();
	return (0);
}
