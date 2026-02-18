#version 430 core
layout (local_size_x = 8, local_size_y = 8) in;
layout (binding = 0, rgba32f) uniform image2D u_output;

uniform vec2 u_resolution;
uniform uint u_mesh_count;

// ------------------------------------------------------------------ structs --

struct s_ray {
    vec3 origin;
    vec3 dir;
};

struct s_triangle {
    vec4 v0;
    vec4 v1;
    vec4 v2;
};

struct s_mesh_descriptor {
    vec4  position;   // xyz = world pos, w = bounding radius
    uint  tri_offset;
    uint  tri_count;
    float pad0;
    float pad1;
};

struct s_hit {
    float t;
    vec3  normal;
    uint  mesh_index;
};

// ------------------------------------------------------------------ SSBOs ---

layout(std430, binding = 1) buffer Triangles { s_triangle triangles[]; };
layout(std430, binding = 2) buffer Meshes    { s_mesh_descriptor meshes[]; };

// ------------------------------------------------------- broad phase cull ---

bool intersect_sphere_bound(s_ray ray, vec3 center, float radius)
{
    vec3  oc = ray.origin - center;
    float b  = dot(oc, ray.dir);
    float c  = dot(oc, oc) - radius * radius;
    float discriminant = b * b - c;
    if (discriminant < 0.0)
        return false;
    float sqrt_d = sqrt(discriminant);
    // either intersection point in front of ray is enough
    return (-b + sqrt_d) > 0.0;
}
// ---------------------------------------------------- Möller–Trumbore -------

bool intersect_triangle(s_ray ray, s_triangle tri, out float t)
{
    const float EPS = 1e-6;

    vec3 v0 = tri.v0.xyz;   // <-- swizzle here
    vec3 v1 = tri.v1.xyz;
    vec3 v2 = tri.v2.xyz;

    vec3  e1   = v1 - v0;
    vec3  e2   = v2 - v0;
    vec3  pvec = cross(ray.dir, e2);
    float det  = dot(e1, pvec);
    if (abs(det) < EPS)
        return false;

    float inv  = 1.0 / det;
    vec3  tvec = ray.origin - v0;
    float u    = dot(tvec, pvec) * inv;
    if (u < 0.0 || u > 1.0)
        return false;

    vec3  qvec = cross(tvec, e1);
    float v    = dot(ray.dir, qvec) * inv;
    if (v < 0.0 || u + v > 1.0)
        return false;

    t = dot(e2, qvec) * inv;
    return t > EPS;
}

// ---------------------------------------------------------- scene traverse --

bool scene_intersect(s_ray ray_world, out s_hit hit)
{
    hit.t = 1e30;
    bool found = false;

    for (uint m = 0; m < u_mesh_count; m++)
    {
        vec3  mesh_pos = meshes[m].position.xyz;  // <-- swizzle here
        float bound_r  = meshes[m].position.w;

        s_ray ray;
        ray.origin = ray_world.origin - mesh_pos;
        ray.dir    = ray_world.dir;

        if (!intersect_sphere_bound(ray, vec3(0.0), bound_r))
            continue;

        uint start = meshes[m].tri_offset;
        uint end   = start + meshes[m].tri_count;

        for (uint i = start; i < end; i++)
        {
            float t;
            if (intersect_triangle(ray, triangles[i], t) && t < hit.t)
            {
                hit.t          = t;
                hit.mesh_index = m;

                vec3 e1 = triangles[i].v1.xyz - triangles[i].v0.xyz;  // <-- swizzle here
                vec3 e2 = triangles[i].v2.xyz - triangles[i].v0.xyz;
                hit.normal = normalize(cross(e1, e2));

                found = true;
            }
        }
    }
    return found;
}

// ------------------------------------------------------------ sky gradient --

vec3 sky_color(vec3 dir)
{
    float t = 0.5 * (dir.y + 1.0);
    return mix(vec3(1.0), vec3(0.5, 0.7, 1.0), t);
}

// --------------------------------------------------------------------- main --

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= int(u_resolution.x) ||
        pixel.y >= int(u_resolution.y))
        return;

    vec2 uv  = (vec2(pixel) + 0.5) / u_resolution * 2.0 - 1.0;
    uv.x    *= u_resolution.x / u_resolution.y;

    s_ray ray;
    ray.origin = vec3(0.0);
    ray.dir    = normalize(vec3(uv, -1.0));

    s_hit hit;
    vec3 color;

    if (scene_intersect(ray, hit))
    {
        vec3 normal = hit.normal * sign(-dot(hit.normal, ray.dir));
        color = normal * 0.5 + 0.5;
    }
    else
        color = sky_color(ray.dir);

    imageStore(u_output, pixel, vec4(color, 1.0));
}
