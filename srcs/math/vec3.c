#include "rt_math.h"
#include <math.h>

t_vec3	vec3(float x, float y, float z)
{
	t_vec3	vec;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	return (vec);
}

t_vec3	vec3_add(t_vec3 a, t_vec3 b)
{
	t_vec3	res;

	res = vec3(a.x + b.x, a.y + b.y, a.z + b.z);
	return (res);
}

t_vec3	vec3_sub(t_vec3 a, t_vec3 b)
{
	t_vec3	res;

	res = vec3(a.x - b.x, a.y - b.y, a.z - b.z);
	return (res);
}

t_vec3	vec3_mul(t_vec3 v, float s)
{
	t_vec3	res;
	
	res = vec3(v.x * s, v.y * s, v.z * s);
	return (res);
}

t_vec3	vec3_div(t_vec3 v, float s)
{
	t_vec3	res;

	res = vec3(v.x / s, v.y / s, v.z / s);
    return (res);
}

float	vec3_dot(t_vec3 a, t_vec3 b)
{
	float	res;

	res = a.x * b.x + a.y * b.y + a.z * b.z;
    return (res);
}

t_vec3	vec3_cross(t_vec3 a, t_vec3 b)
{
	t_vec3	res;

	res = vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
	return (res);
}

float vec3_length(t_vec3 v)
{
	float	res;

	res = sqrtf(vec3_dot(v, v));
    return (res);
}

t_vec3 vec3_normalize(t_vec3 v)
{
    float	len;

	len = vec3_length(v);
    if (len == 0.0f)
        return (vec3(0.0f, 0.0f, 0.0f));
    return (vec3_div(v, len));
}

t_vec3 vec3_reflect(t_vec3 v, t_vec3 n)
{
	t_vec3	res;

	res = vec3_sub(v, vec3_mul(n, 2.0f * vec3_dot(v, n)));
    return (res);
}

