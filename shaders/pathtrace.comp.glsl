#version 430 core

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0, rgba32f) uniform image2D u_output;

uniform vec2 u_resolution;
uniform uint u_triangle_count;
uniform vec3 u_mesh_position;   // <-- NEW

struct s_ray
{
    vec3 origin;
    vec3 dir;
};

struct s_triangle
{
    vec3 v0;
    vec3 v1;
    vec3 v2;
};

layout(std430, binding = 1) buffer Triangles
{
    s_triangle triangles[];
};

bool intersect_triangle(s_ray ray, s_triangle tri, out float t)
{
    const float EPS = 0.000001;

    vec3 edge1 = tri.v1 - tri.v0;
    vec3 edge2 = tri.v2 - tri.v0;

    vec3 pvec = cross(ray.dir, edge2);
    float det = dot(edge1, pvec);

    if (abs(det) < EPS)
        return false;

    float invDet = 1.0 / det;

    vec3 tvec = ray.origin - tri.v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0)
        return false;

    vec3 qvec = cross(tvec, edge1);
    float v = dot(ray.dir, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0)
        return false;

    t = dot(edge2, qvec) * invDet;

    return t > EPS;
}

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    if (pixel.x >= int(u_resolution.x) ||
        pixel.y >= int(u_resolution.y))
        return;

    vec2 uv = (vec2(pixel) + 0.5) / u_resolution;
    uv = uv * 2.0 - 1.0;

    float aspect = u_resolution.x / u_resolution.y;
    uv.x *= aspect;

    // -------- WORLD SPACE RAY --------
    s_ray ray_world;
    ray_world.origin = vec3(0.0, 0.0, 0.0);
    ray_world.dir = normalize(vec3(uv, -1.0));

    // -------- TRANSFORM TO OBJECT SPACE --------
    s_ray ray;
    ray.origin = ray_world.origin - u_mesh_position;
    ray.dir    = ray_world.dir;

    float closest_t = 1e30;
    bool hit = false;
    vec3 hit_normal;

    for (uint i = 0; i < u_triangle_count; i++)
    {
        float t;
        if (intersect_triangle(ray, triangles[i], t))
        {
            if (t < closest_t)
            {
                closest_t = t;
                hit = true;

                vec3 e1 = triangles[i].v1 - triangles[i].v0;
                vec3 e2 = triangles[i].v2 - triangles[i].v0;
                hit_normal = normalize(cross(e1, e2));
            }
        }
    }

    vec3 color;

    if (hit)
        color = hit_normal * 0.5 + 0.5;
    else
    {
        float sky = 0.5 * (ray_world.dir.y + 1.0);
        color = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), sky);
    }

    imageStore(u_output, pixel, vec4(color, 1.0));
}

