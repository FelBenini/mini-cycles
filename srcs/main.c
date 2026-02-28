#include "camera.h"
#include "cycles.h"
#include "input.h"
#include "mesh.h"
#include "rt_math.h"
#include "scene.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdint.h>

#define WIDTH 1920
#define HEIGHT 1080

void	create_balls(t_scene *scene)
{
	int		i;
	int		j;
	t_mesh	ball;

	i = 0;
	while (i < 20)
	{
		j = 0;
		while (j < 10)
		{
			ball = generate_uv_sphere(32, 32, 0.3f);
			ball.position = (t_vec4){(i * 0.8f) - 4.0f, (j * 0.8f) - 4.0f,
				-5.0f, 0.0f};
			scene_add_mesh(scene, ball);
			j++;
		}
		i++;
	}
}

static void	render_frame(t_cycles cycles, GLint loc_resolution,
		GLint loc_mesh_count, GLint loc_accumulation_tex_fs, t_scene scene)
{
	glUseProgram(cycles.compute_program);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.ssbo_triangles);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, scene.ssbo_normals);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.ssbo_meshes);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, scene.ssbo_bvh_nodes);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, scene.ssbo_tlas_nodes);
	glBindImageTexture(0, cycles.tex, 0, GL_FALSE, 0, GL_WRITE_ONLY,
		GL_RGBA32F);
	glUniform2f(loc_resolution, (float)WIDTH, (float)HEIGHT);
	glUniform1ui(loc_mesh_count, scene.mesh_count);
	glDispatchCompute((WIDTH + 7) / 8, (HEIGHT + 7) / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(cycles.fullscreen_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cycles.tex);
	glUniform1i(loc_accumulation_tex_fs, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

int	main(void)
{
	t_cycles		cycles;
	t_scene			scene;
	t_camera		cam;
	t_cam_uniforms	cam_u;
	GLint			loc_resolution;
	GLint			loc_mesh_count;
	GLint			loc_accumulation_tex_fs;

	cycles = init_cycles();
	scene = scene_create(8);
	create_balls(&scene);
	scene_upload_triangles(&scene);
	scene_upload_bvh_nodes(&scene);
	scene_rebuild_tlas(&scene);
	scene_upload_tlas_nodes(&scene);
	cam = camera_create(0.0f, 0.0f, 3.0f, 60.0f);
	glfwSetInputMode(cycles.win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(cycles.win, &cam);
	glfwSetCursorPosCallback(cycles.win, mouse_callback);
	glfwSetScrollCallback(cycles.win, scroll_callback);
	cam_u = get_cam_uniform_locations(cycles.compute_program);
	loc_resolution = glGetUniformLocation(cycles.compute_program,
			"u_resolution");
	loc_mesh_count = glGetUniformLocation(cycles.compute_program,
			"u_mesh_count");
	loc_accumulation_tex_fs = glGetUniformLocation(cycles.fullscreen_program,
			"u_accumulation_tex");
	while (!glfwWindowShouldClose(cycles.win))
	{
		glfwPollEvents();
		handle_input(cycles.win, &cam);
		if (scene.desc_dirty)
			scene_upload_descriptors(&scene);
		if (scene.tlas_dirty)
		{
			scene_rebuild_tlas(&scene);
			scene_upload_tlas_nodes(&scene);
		}
		upload_camera(cycles.compute_program, cam_u, &cam);
		render_frame(cycles, loc_resolution, loc_mesh_count,
			loc_accumulation_tex_fs, scene);
		glfwSwapBuffers(cycles.win);
	}
	scene_destroy(&scene);
	glfwTerminate();
	return (0);
}
