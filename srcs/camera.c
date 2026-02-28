#include "camera.h"
#include "math.h"

t_camera	camera_create(float x, float y, float z, float fov_deg)
{
	t_camera	cam;

	cam.pos   = (t_vec4){x, y, z, 0.0f};
	cam.yaw   = 0.0f;
	cam.pitch = 0.0f;
	cam.fov   = fov_deg * (3.14159265f / 180.0f);
	return (cam);
}

void	camera_update_basis(t_camera *cam)
{
	float	cp = cosf(cam->pitch);
	float	sp = sinf(cam->pitch);
	float	cy = cosf(cam->yaw);
	float	sy = sinf(cam->yaw);

	cam->forward = (t_vec4){ cy * cp,  sp, -sy * cp, 0.0f };
	cam->right = (t_vec4){ sy, 0.0f, cy, 0.0f };
	cam->up = (t_vec4){
		cam->right.y * cam->forward.z - cam->right.z * cam->forward.y,
		-cam->right.x * cam->forward.z + cam->right.z * cam->forward.x,
		cam->right.x * cam->forward.y - cam->right.y * cam->forward.x,
		0.0f
	};
}

t_cam_uniforms	get_cam_uniform_locations(GLuint program)
{
	t_cam_uniforms	u;

	u.pos = glGetUniformLocation(program, "u_cam_pos");
	u.forward = glGetUniformLocation(program, "u_cam_forward");
	u.right = glGetUniformLocation(program, "u_cam_right");
	u.up = glGetUniformLocation(program, "u_cam_up");
	u.fov = glGetUniformLocation(program, "u_cam_fov");
	return (u);
}

void	upload_camera(GLuint program, t_cam_uniforms u, t_camera *cam)
{
	camera_update_basis(cam);
	glUseProgram(program);
	glUniform3f(u.pos, cam->pos.x, cam->pos.y, cam->pos.z);
	glUniform3f(u.forward, cam->forward.x, cam->forward.y, cam->forward.z);
	glUniform3f(u.right, cam->right.x, cam->right.y, cam->right.z);
	glUniform3f(u.up, cam->up.x, cam->up.y, cam->up.z);
	glUniform1f(u.fov, cam->fov);
}
