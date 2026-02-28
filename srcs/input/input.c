#include "input.h"
#include "camera.h"

void	handle_input(GLFWwindow *win, t_camera *cam)
{
	if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
	{
		cam->pos.x += cam->forward.x * CAM_SPEED;
		cam->pos.y += cam->forward.y * CAM_SPEED;
		cam->pos.z += cam->forward.z * CAM_SPEED;
	}
	if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
	{
		cam->pos.x -= cam->forward.x * CAM_SPEED;
		cam->pos.y -= cam->forward.y * CAM_SPEED;
		cam->pos.z -= cam->forward.z * CAM_SPEED;
	}
	if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
	{
		cam->pos.x -= cam->right.x * CAM_SPEED;
		cam->pos.z -= cam->right.z * CAM_SPEED;
	}
	if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
	{
		cam->pos.x += cam->right.x * CAM_SPEED;
		cam->pos.z += cam->right.z * CAM_SPEED;
	}
	if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
		cam->pos.y += CAM_SPEED;
	if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cam->pos.y -= CAM_SPEED;
	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
