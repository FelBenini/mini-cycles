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

