#include "rt_math.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

int	main(void)
{
	t_vec3	a;
	t_vec3	b;
	t_vec3	c;
	a = vec3(1, 1, 1);
	b = vec3(2, 1, 3);
	c = vec3_cross(a, b);
	printf("%f %f %f\n", c.x, c.y, c.z);
}
