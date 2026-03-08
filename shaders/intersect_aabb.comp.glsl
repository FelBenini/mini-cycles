bool intersect_aabb(
    s_ray ray,
    vec3  bmin,
    vec3  bmax,
    float max_t,
    out float t_near)
{
    vec3 t0 = (bmin - ray.origin) * ray.inv_dir;
    vec3 t1 = (bmax - ray.origin) * ray.inv_dir;

    vec3 tmin_v = min(t0, t1);
    vec3 tmax_v = max(t0, t1);

    float t0_max = max(max(tmin_v.x, tmin_v.y), tmin_v.z);
    float t1_min = min(min(tmax_v.x, tmax_v.y), tmax_v.z);

    t_near = t0_max;

    return (t1_min >= t0_max) &&
           (t1_min >  1e-4)   &&
           (t0_max <  max_t);
}
