#version 430 core
layout (local_size_x = 8, local_size_y = 8) in;
layout (binding = 0, rgba32f) uniform image2D u_output;

uniform vec2 u_resolution;

// ------------------------------------------------------------------ structs --

struct s_ray {
    vec3 origin;
    vec3 dir;
    vec3 inv_dir;   // precomputed reciprocal — avoids per-node division
};

struct s_triangle {
    vec4 v0;
    vec4 v1;
    vec4 v2;
};

struct s_triangle_normals {
    vec4 n0;
    vec4 n1;
    vec4 n2;
};

struct s_mesh_descriptor {
    vec4  position;
    uint  tri_offset;
    uint  tri_count;
    uint  smooth_shade;
    uint  bvh_root;
};

struct s_bvh_node {
    vec4  bbox_min;
    vec4  bbox_max;
    uint  left_child;
    uint  right_child;
    uint  tri_offset;
    uint  tri_count;
};

struct s_tlas_node {
    vec4  bbox_min;
    vec4  bbox_max;
    uint  left_child;
    uint  right_child;
    uint  mesh_index;
    uint  _padding;
};

struct s_hit {
    float t;
    vec3  pos;
    vec3  normal;
    vec3  geo_normal;
    uint  mesh_index;
};

// ------------------------------------------------------------------ SSBOs --
// readonly lets the driver skip tracking stores — measurable on large BVHs

layout(std430, binding = 1) readonly buffer Triangles { s_triangle triangles[]; };
layout(std430, binding = 2) readonly buffer TriangleNormals { s_triangle_normals triangle_normals[]; };
layout(std430, binding = 3) readonly buffer Meshes    { s_mesh_descriptor meshes[]; };
layout(std430, binding = 4) readonly buffer BVHNodes  { s_bvh_node bvh_nodes[]; };
layout(std430, binding = 5) readonly buffer TLASNodes { s_tlas_node tlas_nodes[]; };

// --------------------------------------------------------- AABB
// Uses precomputed inv_dir passed in via the ray struct.

bool intersect_aabb(
    s_ray  ray,
    vec3   bmin,
    vec3   bmax,
    float  max_t,
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
           (t1_min >  0.0)    &&
           (t0_max <  max_t);
}

// --------------------------------------------------------- triangle

bool intersect_triangle(
    s_ray       ray,
    s_triangle  tri,
    out float   t,
    out vec3    bary)
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

// --------------------------------------------------------- BLAS traversal (closest hit)
// Ordered: both children are tested, closer one is pushed last so it's popped first.

void blas_intersect(
    s_ray         ray,
    uint          mesh_idx,
    inout s_hit   hit)
{
    uint stack[64];
    uint ptr = 0;
    stack[ptr++] = meshes[mesh_idx].bvh_root;

    while (ptr > 0)
    {
        uint       node_idx = stack[--ptr];
        s_bvh_node node     = bvh_nodes[node_idx];

        float tnear;
        if (!intersect_aabb(ray,
                            node.bbox_min.xyz,
                            node.bbox_max.xyz,
                            hit.t,
                            tnear))
            continue;

        // ---- leaf ----
        if (node.tri_count > 0)
        {
            uint base = meshes[mesh_idx].tri_offset + node.tri_offset;
            for (uint i = 0; i < node.tri_count; i++)
            {
                float t;
                vec3  bary;
                uint  tri_idx = base + i;

                if (intersect_triangle(ray, triangles[tri_idx], t, bary)
                    && t < hit.t)
                {
                    hit.t          = t;
                    hit.mesh_index = mesh_idx;

                    vec3 e1    = triangles[tri_idx].v1.xyz - triangles[tri_idx].v0.xyz;
                    vec3 e2    = triangles[tri_idx].v2.xyz - triangles[tri_idx].v0.xyz;
                    vec3 geo_n = normalize(cross(e1, e2));
                    if (dot(geo_n, ray.dir) > 0.0)
                        geo_n = -geo_n;

                    hit.geo_normal = geo_n;

                    if (meshes[mesh_idx].smooth_shade == 1u)
                    {
                        vec3 interp = bary.x * triangle_normals[tri_idx].n0.xyz
                                    + bary.y * triangle_normals[tri_idx].n1.xyz
                                    + bary.z * triangle_normals[tri_idx].n2.xyz;
                        hit.normal = normalize(interp);
                        if (dot(hit.normal, geo_n) < 0.0)
                            hit.normal = -hit.normal;
                    }
                    else
                        hit.normal = geo_n;
                }
            }
            continue;
        }

        // ---- interior: ordered push ----
        // Test both children, push farther first so closer is on top of stack.
        float t_left  = 1e30;
        float t_right = 1e30;
        bool  hit_l   = false;
        bool  hit_r   = false;

        if (node.left_child != 0)
            hit_l = intersect_aabb(ray,
                                   bvh_nodes[node.left_child].bbox_min.xyz,
                                   bvh_nodes[node.left_child].bbox_max.xyz,
                                   hit.t, t_left);
        if (node.right_child != 0)
            hit_r = intersect_aabb(ray,
                                   bvh_nodes[node.right_child].bbox_min.xyz,
                                   bvh_nodes[node.right_child].bbox_max.xyz,
                                   hit.t, t_right);

        if (hit_l && hit_r)
        {
            // push farther child first — closer child ends up on top
            if (t_left <= t_right)
            {
                stack[ptr++] = node.right_child;
                stack[ptr++] = node.left_child;
            }
            else
            {
                stack[ptr++] = node.left_child;
                stack[ptr++] = node.right_child;
            }
        }
        else if (hit_l) stack[ptr++] = node.left_child;
        else if (hit_r) stack[ptr++] = node.right_child;
    }
}

// --------------------------------------------------------- BLAS traversal (any hit / shadow)
// Returns true immediately on the first triangle intersection within max_t.

bool blas_intersect_any(
    s_ray  ray,
    uint   mesh_idx,
    float  max_t)
{
    uint stack[32];
    uint ptr = 0;
    stack[ptr++] = meshes[mesh_idx].bvh_root;

    while (ptr > 0)
    {
        uint       node_idx = stack[--ptr];
        s_bvh_node node     = bvh_nodes[node_idx];

        float tnear;
        if (!intersect_aabb(ray, node.bbox_min.xyz, node.bbox_max.xyz, max_t, tnear))
            continue;

        if (node.tri_count > 0)
        {
            uint base = meshes[mesh_idx].tri_offset + node.tri_offset;
            for (uint i = 0; i < node.tri_count; i++)
            {
                float t;
                vec3  bary;
                if (intersect_triangle(ray, triangles[base + i], t, bary)
                    && t < max_t)
                    return true;   // early exit — no need to find closest
            }
            continue;
        }

        if (node.right_child != 0) stack[ptr++] = node.right_child;
        if (node.left_child  != 0) stack[ptr++] = node.left_child;
    }
    return false;
}

// --------------------------------------------------------- TLAS traversal (closest hit)

bool scene_intersect(s_ray ray_world, out s_hit hit)
{
    hit.t  = 1e30;
    bool found = false;

    uint tlas_stack[64];
    uint tlas_ptr = 0;
    tlas_stack[tlas_ptr++] = 0;

    while (tlas_ptr > 0)
    {
        uint        tlas_idx = tlas_stack[--tlas_ptr];
        s_tlas_node tnode    = tlas_nodes[tlas_idx];

        float tnear_tlas;
        if (!intersect_aabb(ray_world,
                            tnode.bbox_min.xyz,
                            tnode.bbox_max.xyz,
                            hit.t,
                            tnear_tlas))
            continue;

        // ---- leaf: descend into BLAS ----
        if (tnode.left_child == 0 && tnode.right_child == 0)
        {
            uint  mesh_idx = tnode.mesh_index;
            vec3  mesh_pos = meshes[mesh_idx].position.xyz;

            s_ray ray;
            ray.origin  = ray_world.origin - mesh_pos;
            ray.dir     = ray_world.dir;
            ray.inv_dir = ray_world.inv_dir;  // direction unchanged, inv_dir reusable

            uint prev_tri_count = 0;        // track whether BLAS found something
            float t_before = hit.t;

            blas_intersect(ray, mesh_idx, hit);

            if (hit.t < t_before)
                found = true;
            continue;
        }

        // ---- interior: ordered TLAS push ----
        float t_left  = 1e30;
        float t_right = 1e30;
        bool  hit_l   = false;
        bool  hit_r   = false;

        if (tnode.left_child != 0)
            hit_l = intersect_aabb(ray_world,
                                   tlas_nodes[tnode.left_child].bbox_min.xyz,
                                   tlas_nodes[tnode.left_child].bbox_max.xyz,
                                   hit.t, t_left);
        if (tnode.right_child != 0)
            hit_r = intersect_aabb(ray_world,
                                   tlas_nodes[tnode.right_child].bbox_min.xyz,
                                   tlas_nodes[tnode.right_child].bbox_max.xyz,
                                   hit.t, t_right);

        if (hit_l && hit_r)
        {
            if (t_left <= t_right)
            {
                tlas_stack[tlas_ptr++] = tnode.right_child;
                tlas_stack[tlas_ptr++] = tnode.left_child;
            }
            else
            {
                tlas_stack[tlas_ptr++] = tnode.left_child;
                tlas_stack[tlas_ptr++] = tnode.right_child;
            }
        }
        else if (hit_l) tlas_stack[tlas_ptr++] = tnode.left_child;
        else if (hit_r) tlas_stack[tlas_ptr++] = tnode.right_child;
    }

    if (found)
        hit.pos = ray_world.origin + ray_world.dir * hit.t;

    return found;
}

// --------------------------------------------------------- TLAS traversal (any hit / shadow)

bool scene_occluded(vec3 origin, vec3 dir, float max_t)
{
    s_ray r;
    r.origin  = origin;
    r.dir     = dir;
    r.inv_dir = 1.0 / dir;

    uint tlas_stack[64];
    uint tlas_ptr = 0;
    tlas_stack[tlas_ptr++] = 0;

    while (tlas_ptr > 0)
    {
        uint        tlas_idx = tlas_stack[--tlas_ptr];
        s_tlas_node tnode    = tlas_nodes[tlas_idx];

        float tnear;
        if (!intersect_aabb(r,
                            tnode.bbox_min.xyz,
                            tnode.bbox_max.xyz,
                            max_t, tnear))
            continue;

        if (tnode.left_child == 0 && tnode.right_child == 0)
        {
            uint  mesh_idx = tnode.mesh_index;
            vec3  mesh_pos = meshes[mesh_idx].position.xyz;

            s_ray lr;
            lr.origin  = r.origin - mesh_pos;
            lr.dir     = r.dir;
            lr.inv_dir = r.inv_dir;

            if (blas_intersect_any(lr, mesh_idx, max_t))
                return true;    // early exit — fully skips remaining TLAS nodes
            continue;
        }

        if (tnode.right_child != 0) tlas_stack[tlas_ptr++] = tnode.right_child;
        if (tnode.left_child  != 0) tlas_stack[tlas_ptr++] = tnode.left_child;
    }
    return false;
}

// --------------------------------------------------------- shading

const vec3  SUN_DIR       = normalize(vec3(-0.6, 1.0, 0.4));
const vec3  SUN_COLOR     = vec3(1.0);
const vec3  AMBIENT_COLOR = vec3(0.1, 0.12, 0.18);
const float SPECULAR_POW  = 32.0;
const float SHADOW_BIAS   = 1e-3;
const float SUN_DIST      = 1e10; // effectively infinite — used as shadow max_t

vec3 shade(s_hit hit, s_ray camera_ray)
{
    vec3 N = hit.normal;
    vec3 L = SUN_DIR;
    vec3 V = -camera_ray.dir;
    vec3 H = normalize(L + V);

    vec3  shadow_origin = hit.pos + hit.geo_normal * SHADOW_BIAS;
    float shadow = scene_occluded(shadow_origin, L, SUN_DIST) ? 0.0 : 1.0;

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), SPECULAR_POW);

    return AMBIENT_COLOR
         + shadow * SUN_COLOR * diff
         + shadow * SUN_COLOR * spec * 0.4;
}

vec3 sky_color(vec3 dir)
{
    float t = 0.5 * (dir.y + 1.0);
    return mix(vec3(1.0), vec3(0.5, 0.7, 1.0), t);
}

// --------------------------------------------------------- main

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= int(u_resolution.x) ||
        pixel.y >= int(u_resolution.y))
        return;

    vec2 uv = (vec2(pixel) + 0.5) / u_resolution * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    s_ray ray;
    ray.origin  = vec3(0.0);
    ray.dir     = normalize(vec3(uv, -1.0));
    ray.inv_dir = 1.0 / ray.dir;   // computed once, reused everywhere

    s_hit hit;
    vec3  color;

    if (scene_intersect(ray, hit))
        color = shade(hit, ray);
    else
        color = sky_color(ray.dir);

    imageStore(u_output, pixel, vec4(color, 1.0));
}
