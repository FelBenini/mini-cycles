#include "cycles.h"
#include "camera.h"

void	mouse_callback(GLFWwindow *win, double x, double y)
{
	static int		first_move = 1;
	static double	last_x;
	static double	last_y;
	t_camera		*cam;

	cam = glfwGetWindowUserPointer(win);
	if (first_move)
	{
		last_x = x;
        last_y = y;
		first_move = 0;
        return ;
    }
	float dx =  (float)(x - last_x) * CAM_SENSITIVITY;
	float dy = -(float)(y - last_y) * CAM_SENSITIVITY;
	last_x = x;
	last_y = y;
	cam->yaw   -= dx;
	cam->pitch += dy;
	if (cam->pitch >  1.5f)
		cam->pitch =  1.5f;
	if (cam->pitch < -1.5f)
		cam->pitch = -1.5f;
}

void	scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
{
	t_camera	*cam;

	cam = glfwGetWindowUserPointer(win);
	(void)xoffset;
	cam->fov -= (float)yoffset * 0.05f;
	if (cam->fov < 0.17f)
		cam->fov = 0.17f;
	if (cam->fov > 2.62f)
		cam->fov = 2.62f;
}
