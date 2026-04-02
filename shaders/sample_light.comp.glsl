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

vec3 sample_emissive_meshes(vec3 pos, vec3 normal, float bias, inout uint seed)
{
    vec3 result = vec3(0.0);
    uint mesh_count = meshes.length();
    if (mesh_count == 0u)
        return result;

    for (uint mesh_idx = 0u; mesh_idx < mesh_count; mesh_idx++)
    {
        s_mesh_descriptor mesh = meshes[mesh_idx];
        s_material        mat  = materials[mesh.material];

        if (length(mat.emission.rgb) <= 0.0)
            continue;

        uint tri_count  = mesh.tri_count;
        uint tri_offset = mesh.tri_offset;
        if (tri_count == 0u)
            continue;

        // ─ 1. Uniform random triangle
        float r0      = rand(seed);
        uint  tri_idx = tri_offset + min(uint(r0 * float(tri_count)), tri_count - 1u);
        // pdf_tri = 1 / tri_count  (uniform discrete)

        s_triangle tri = triangles[tri_idx];

        // ─ 2. Uniform random point on the triangle (correct barycentric)
        //   Use the standard sqrt-based mapping so the density is uniform in area.
        float r1      = rand(seed);
        float r2      = rand(seed);
        float sqrt_r1 = sqrt(r1);
        float u       = 1.0 - sqrt_r1;           // barycentric weight for v0
        float v       = sqrt_r1 * (1.0 - r2);    // barycentric weight for v1
        float w       = sqrt_r1 * r2;             // barycentric weight for v2

        vec3 light_pos = tri.v0.xyz * u
                       + tri.v1.xyz * v
                       + tri.v2.xyz * w;

        // ─ 3. Triangle area and surface normal
        vec3  e0        = tri.v1.xyz - tri.v0.xyz;
        vec3  e1        = tri.v2.xyz - tri.v0.xyz;
        vec3  cross_e   = cross(e0, e1);
        float tri_area  = 0.5 * length(cross_e);
        if (tri_area <= 0.0)
            continue;
        vec3  light_n   = cross_e / (2.0 * tri_area); // normalised light normal

        // ─ 4. Geometry vectors
        vec3  origin    = pos + normal * bias;
        vec3  to_light  = light_pos - origin;
        float dist2     = dot(to_light, to_light);
        float dist      = sqrt(dist2);
        vec3  L         = to_light / dist;

        // ─ 5. Cosines
        float NdotL       = dot(normal,   L);        // receiver cosine
        float LdotLN      = dot(light_n, -L);        // emitter cosine (back-face)

        if (NdotL <= 0.0 || LdotLN <= 0.0)
            continue;

        // ─ 6. Visibility
        s_ray shadow_ray;
        shadow_ray.origin  = origin;
        shadow_ray.dir     = L;
        shadow_ray.inv_dir = 1.0 / L;

        s_hit shadow_hit;
        if (scene_intersect(shadow_ray, shadow_hit) && shadow_hit.t < dist - bias)
            continue;

        // ─ 7. PDF ( probability density function) in area measure, then convert to solid-angle measure
        //   pdf_area   = 1 / (tri_count * tri_area)
        //   pdf_solid  = pdf_area * dist² / LdotLN
        //   1/pdf_solid = (tri_count * tri_area * LdotLN) / dist²
        float inv_pdf = (float(tri_count) * tri_area * LdotLN) / dist2;

        // ─ 8. Accumulate: Le * NdotL / pdf_solid
        result += mat.emission.rgb * NdotL * inv_pdf;
    }
    return result;
}
