#ifndef CAMERA_H
# define CAMERA_H

# include "rt_math.h"

typedef struct s_camera
{
	t_vec4	pos;
	t_vec4	lower_left_corner;
	t_vec4	horizontal;
	t_vec4	vertical;
}	t_camera;

#endif
