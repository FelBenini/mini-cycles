#include "cycles.h"
#include "camera.h"
#include <GLFW/glfw3.h>

static int	g_first_move = 0;
static int	g_mouse_look = 0;

void	mouse_callback(GLFWwindow *win, double x, double y)
{
	static double	last_x;
	static double	last_y;
	t_camera		*cam;

	cam = glfwGetWindowUserPointer(win);
	if (!g_mouse_look)
	{
		g_first_move = 1;
		return ;
	}
	if (g_first_move)
	{
		last_x = x;
        last_y = y;
		g_first_move = 0;
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

void	mouse_button_callback(GLFWwindow *win, int button, int action, int mods)
{
    (void)win;
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
		if (action == GLFW_PRESS)
		{
			g_first_move = 1;
			g_mouse_look = 1;
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else if (action == GLFW_RELEASE)
		{
			g_mouse_look = 0;
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}
