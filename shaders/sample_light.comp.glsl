vec3 sample_emissive_mesh(vec3 pos, vec3 normal, float bias, inout uint seed)
{
    if (u_emissive_mesh_count == 0u)
        return vec3(0.0);

    // Pick a random emissive mesh
    uint em_idx = min(uint(rand(seed) * float(u_emissive_mesh_count)),
                      u_emissive_mesh_count - 1u);
    uint mesh_idx = emissive_mesh_indices[em_idx];

    s_mesh_descriptor mesh = meshes[mesh_idx];
    s_material        mat  = materials[mesh.material];

    // Pick a random triangle within the mesh
    uint tri_local = min(uint(rand(seed) * float(mesh.tri_count)),
                         mesh.tri_count - 1u);
    uint tri_idx   = mesh.tri_offset + tri_local;

    // Sample a random point on the triangle (uniform barycentric)
    float u = rand(seed);
    float v = rand(seed);
    if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }
    float w = 1.0 - u - v;

    s_triangle tri = triangles[tri_idx];
    vec3 local_pos = w * tri.v0.xyz + u * tri.v1.xyz + v * tri.v2.xyz;

    // Triangle edges and geometric normal / area (local space)
    vec3  e1      = tri.v1.xyz - tri.v0.xyz;
    vec3  e2      = tri.v2.xyz - tri.v0.xyz;
    vec3  cross_e = cross(e1, e2);
    float area    = length(cross_e) * 0.5;
    if (area < 1e-8)
        return vec3(0.0);
    vec3  light_n = normalize(cross_e);

    // Transform sampled point and normal to world space
    mat3 R            = mat_from_dir(mesh.direction.xyz);
    vec3 world_point  = R * local_pos + mesh.position.xyz;
    vec3 world_light_n = normalize(R * light_n);

    // Direction and distance from shading point to light sample
    vec3  to_light = world_point - pos;
    float dist2    = dot(to_light, to_light);
    if (dist2 < 1e-8)
        return vec3(0.0);
    float dist = sqrt(dist2);
    vec3  L    = to_light / dist;

    // Shading-side cosine
    float NdotL = dot(normal, L);
    if (NdotL <= 0.0)
        return vec3(0.0);

    // Light-side cosine (use abs so both triangle faces emit)
    float cos_light = abs(dot(world_light_n, -L));
    if (cos_light <= 0.0)
        return vec3(0.0);

    // Shadow ray
    s_ray shadow_ray;
    shadow_ray.origin  = pos + normal * bias;
    shadow_ray.dir     = L;
    shadow_ray.inv_dir = 1.0 / L;

    s_hit shadow_hit;
    if (scene_intersect(shadow_ray, shadow_hit)
        && shadow_hit.t < dist - bias * 2.0)
        return vec3(0.0);

    // PDF_area = 1 / (N_emissive_meshes * tri_count * area)
    // Convert to solid-angle: pdf_omega = pdf_area * dist^2 / cos_light
    // Contribution = emission * NdotL / pdf_omega
    //              = emission * NdotL * cos_light * area
    //                * N_emissive_meshes * tri_count / dist^2
    float weight = cos_light * area
                 * float(u_emissive_mesh_count)
                 * float(mesh.tri_count)
                 / dist2;

    return mat.emission.rgb * NdotL * weight;
}

vec3 sample_lights(vec3 pos, vec3 normal, float bias)
{
    vec3 result = vec3(0.0);
    for (uint i = 0u; i < u_light_count; i++)
    {
        s_light light = lights[i];

        if (light.type == LIGHT_SUN)
        {
            vec3 L = normalize(light.direction.xyz);
            float NdotL = max(dot(normal, L), 0.0);
            if (NdotL <= 0.0)
				continue;

            s_ray shadow_ray;
            shadow_ray.origin  = pos + normal * bias;
            shadow_ray.dir     = L;
            shadow_ray.inv_dir = 1.0 / L;

            s_hit shadow_hit;
            if (scene_intersect(shadow_ray, shadow_hit))
                continue;

            result += light.color.xyz * light.intensity * NdotL;
        }
        else if (light.type == LIGHT_POINT)
        {
            vec3  to_light    = light.position.xyz - pos;
            float dist        = length(to_light);
            vec3  L           = to_light / dist;
            float NdotL       = max(dot(normal, L), 0.0);
            if (NdotL <= 0.0)
				continue;

            float attenuation = 1.0 / (dist * dist);

            s_ray shadow_ray;
            shadow_ray.origin  = pos + normal * bias;
            shadow_ray.dir     = L;
            shadow_ray.inv_dir = 1.0 / L;

            s_hit shadow_hit;
            if (scene_intersect(shadow_ray, shadow_hit) && shadow_hit.t < dist)
                continue;

            result += light.color.xyz * light.intensity * NdotL * attenuation;
        }
        else if (light.type == LIGHT_SPOT)
        {
            vec3  to_light    = light.position.xyz - pos;
            float dist        = length(to_light);
            vec3  L           = to_light / dist;
            float NdotL       = max(dot(normal, L), 0.0);
            if (NdotL <= 0.0)
				continue;

            vec3  spot_dir    = normalize(-light.direction.xyz);
            float cos_theta   = dot(L, spot_dir);

            float cos_inner   = light.cos_inner;
            float cos_outer   = light.cos_outer;

            if (cos_theta < cos_outer)
				continue;

            float spot_factor = clamp(
                (cos_theta - cos_outer) / (cos_inner - cos_outer),
                0.0, 1.0
            );
            spot_factor = spot_factor * spot_factor;

            float attenuation = 1.0 / (dist * dist);

            s_ray shadow_ray;
            shadow_ray.origin  = pos + normal * bias;
            shadow_ray.dir     = L;
            shadow_ray.inv_dir = 1.0 / L;

            s_hit shadow_hit;
            if (scene_intersect(shadow_ray, shadow_hit) && shadow_hit.t < dist)
                continue;

            result += light.color.xyz * light.intensity * NdotL * attenuation * spot_factor;
        }
    }
    return result;
}

