#include "camera.h"
#include "cycles.h"
#include "rt_math.h"
#include "scene.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include "parser.h"
#include "input.h"
#include "screenshot.h"

static void	render_frame(
	t_cycles cycles,
	t_compute_uniforms compute_u,
	t_fragment_uniforms fragment_u,
	t_scene scene,
	uint32_t frame_index,
	uint32_t reset_samples,
	int preview)
{
	GLuint	tex;
	int		render_width;
	int		render_height;
	int		tile_width = 128;
	int		tile_height = 128;

	tex = preview ? cycles.preview_tex : cycles.tex;
	render_width = preview ? cycles.preview_width : cycles.width;
	render_height = preview ? cycles.preview_height : cycles.height;

	glUseProgram(cycles.compute_program);

	glUniform4f(compute_u.loc_ambient_color, scene.ambient.x, scene.ambient.y, scene.ambient.z, scene.ambient.w);

	glBindImageTexture(0, tex, 0, GL_FALSE, 0,
		GL_READ_WRITE, GL_RGBA32F);
	glUniform2f(compute_u.loc_resolution, (float)render_width, (float)render_height);
	glUniform1ui(compute_u.loc_mesh_count, scene.mesh_count);
	glUniform1i(compute_u.loc_sky_tex, scene.sky_tex);
	glUniform1f(compute_u.loc_sky_intensity, scene.sky_intensity);
	glUniform1ui(compute_u.loc_frame_index, frame_index);
	glUniform1ui(compute_u.loc_reset_samples, reset_samples);
	glUniform1ui(compute_u.loc_light_count, scene.light_count);
	glUniform1ui(compute_u.loc_emissive_mesh_count, scene.emissive_mesh_count);
	glUniform1i(compute_u.loc_max_bounces, preview ? 3 : 6);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	for (int tile_y = 0; tile_y < render_height; tile_y += tile_height)
	{
		for (int tile_x = 0; tile_x < render_width; tile_x += tile_width)
		{
			int tile_w = (tile_x + tile_width > render_width) ? render_width - tile_x : tile_width;
			int tile_h = (tile_y + tile_height > render_height) ? render_height - tile_y : tile_height;

			glUniform2f(compute_u.loc_tile_offset, (float)tile_x, (float)tile_y);
			glDispatchCompute((tile_w + 7) / 8, (tile_h + 7) / 8, 1);
			glFinish();
		}
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glViewport(0, 0, cycles.width, cycles.height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(cycles.fullscreen_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1ui(fragment_u.loc_accumulation_tex_fs, 0);
	glUniform1ui(fragment_u.loc_tonemap_fs, cycles.tonemap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, cycles.lut_tex);
	glUniform1i(fragment_u.loc_lut_tex_fs, 1);
	glUniform1i(fragment_u.loc_lut_size_fs, cycles.lut_size);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void	register_callbacks(t_cycles cycles, t_camera *cam)
{
	(void)cam;
	glfwSetCursorPosCallback(cycles.win, mouse_callback);
	glfwSetScrollCallback(cycles.win, scroll_callback);
	glfwSetMouseButtonCallback(cycles.win, mouse_button_callback);
}

	int	main(int argc, char *argv[])
{
	t_cycles			cycles;
	t_scene				scene;
	t_cam_uniforms		cam_u;
	t_all_uniforms		all_u;
	char				title[126];

	uint32_t frame_index = 0;
	uint32_t preview_frame_index = 0;
	uint32_t reset_samples = 1;
	int was_preview = 1;

	if (argc < 2)
	{
		printf("Please, pass a file as an argument.\n");
		return (1);
	}
	cycles = init_cycles();

	scene = parse_scene(argv[1], &cycles);

	scene_upload_images(&scene);
	scene_upload_triangles(&scene);
	scene_upload_materials(&scene);
	scene_upload_bvh_nodes(&scene);
	scene_rebuild_tlas(&scene);
	scene_upload_tlas_nodes(&scene);
	scene_upload_lights(&scene);
	scene_upload_emissive_meshes(&scene);
	cycles.cam = &scene.camera;
	glfwSetWindowUserPointer(cycles.win, &cycles);
	register_callbacks(cycles, &scene.camera);
	cam_u = get_cam_uniform_locations(cycles.compute_program);
	all_u = get_all_uniform_locations(cycles.compute_program, cycles.fullscreen_program);
	glfwShowWindow(cycles.win);
	while (!glfwWindowShouldClose(cycles.win))
	{
		if (glfwGetKey(cycles.win, GLFW_KEY_P) == GLFW_PRESS)
			save_screenshot(cycles.width, cycles.height);
		glfwSwapBuffers(cycles.win);
		glfwPollEvents();
		handle_input(cycles.win, &scene.camera);
		if (scene.desc_dirty)
			scene_upload_descriptors(&scene);
		if (scene.material_dirty)
		{
			scene_upload_materials(&scene);
			scene_upload_emissive_meshes(&scene);
		}
		if (scene.emissive_mesh_dirty)
			scene_upload_emissive_meshes(&scene);
		if (scene.tlas_dirty)
		{
			scene_rebuild_tlas(&scene);
			scene_upload_tlas_nodes(&scene);
		}
		cycles.preview = scene.camera.dirty || cycles.dirty;
		if (cycles.preview || was_preview != cycles.preview)
		{
			frame_index = 0;
			preview_frame_index = 0;
			reset_samples = 1;
		}
		else
			reset_samples = 0;
		upload_camera(cycles.compute_program, cam_u, &scene.camera);
		was_preview = cycles.preview;
		render_frame(
			cycles,
			all_u.compute,
			all_u.fragment,
			scene,
			cycles.preview ? preview_frame_index : frame_index,
			reset_samples,
			cycles.preview);
		if (cycles.preview)
			preview_frame_index++;
		else
			frame_index++;
		snprintf(title, sizeof(title), "miniCycles | sample %u", frame_index);
		glfwSetWindowTitle(cycles.win, title);
		scene.camera.dirty = 0;
		cycles.dirty = 0;
	}
	scene_destroy(&scene);
	glfwTerminate();
	return (0);
}
