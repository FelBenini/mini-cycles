bool intersect_triangle(
    s_ray      ray,
    s_triangle tri,
    out float  t,
    out vec3   bary)
{
    const float EPS = 1e-6;

    vec3 v0 = tri.v0.xyz;
    vec3 e1 = tri.v1.xyz - v0;
    vec3 e2 = tri.v2.xyz - v0;

    vec3  p   = cross(ray.dir, e2);
    float det = dot(e1, p);
    if (abs(det) < EPS) return false;

    float inv = 1.0 / det;
    vec3  s   = ray.origin - v0;

    float u = dot(s, p) * inv;
    if (u < 0.0 || u > 1.0) return false;

    vec3  q = cross(s, e1);
    float v = dot(ray.dir, q) * inv;
    if (v < 0.0 || u + v > 1.0) return false;

    t = dot(e2, q) * inv;
    if (t < EPS) return false;

    bary = vec3(1.0 - u - v, u, v);
    return true;
}
