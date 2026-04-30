void trace_textures(
    s_material mat,
    inout vec3 N,
    inout s_hit hit,
    inout vec3 albedo,
	inout float alpha,
    inout float rough
)
{
    float uv_scale = mat.texture_tile_size;
    vec2 uv = fract(hit.uv * uv_scale);
    if (mat.texture_idx != -1)
    {
        vec4 tex_color = sample_image(uint(mat.texture_idx), uv);
        albedo = tex_color.rgb;
		alpha = tex_color.a;
    }
    if (mat.texture_displacement_idx != -1)
    {
        uint disp_idx = uint(mat.texture_displacement_idx);
        s_image_meta meta = img_info[disp_idx];
        vec2 texel = vec2(1.0 / float(meta.width), 1.0 / float(meta.height));
        float hL = sample_image(disp_idx, uv - vec2(texel.x, 0.0)).r;
        float hR = sample_image(disp_idx, uv + vec2(texel.x, 0.0)).r;
        float hD = sample_image(disp_idx, uv - vec2(0.0, texel.y)).r;
        float hU = sample_image(disp_idx, uv + vec2(0.0, texel.y)).r;
        float dHdU = (hR - hL) * 0.5;
        float dHdV = (hU - hD) * 0.5;
        vec3 T = normalize(abs(N.x) > 0.1
            ? cross(vec3(0,1,0), N)
            : cross(vec3(1,0,0), N));
        vec3 B = normalize(cross(N, T));
        float scale = 0.02;
        N = normalize(N - scale * (dHdU * T + dHdV * B));
    }
    if (mat.normal_tex_idx != -1)
    {
        vec3 normal_sample = sample_image(uint(mat.normal_tex_idx), uv).rgb;
        vec3 tangent_normal = normalize(normal_sample * 2.0 - 1.0);
        vec3 T = normalize(abs(N.x) > 0.1
            ? cross(vec3(0,1,0), N)
            : cross(vec3(1,0,0), N));
        vec3 B = normalize(cross(N, T));
        N = normalize(T * tangent_normal.x + B * tangent_normal.y + N * tangent_normal.z);
    }
    if (mat.roughness_tex_idx != -1)
    {
        vec4 rough_tex = sample_image(uint(mat.roughness_tex_idx), uv);
        rough = rough_tex.r;
    }
}

vec3 trace_path(s_ray ray, inout uint seed)
{
    vec3 throughput  = vec3(1.0);
    vec3 radiance    = vec3(0.0);

    bool  prev_specular = true;
    float prev_bsdf_pdf = 0.0;
    vec3  prev_origin   = ray.origin;

    for (int bounce = 0; bounce < u_max_bounces; bounce++)
    {
        s_hit hit;

        if (!scene_intersect(ray, hit))
        {
            radiance += throughput * sky_color(ray);
            break;
        }

        s_mesh_descriptor mesh = meshes[hit.mesh_index];
        s_material        mat  = materials[mesh.material];
        float adaptive_bias    = max(1e-4, hit.t * 1e-4);

        // Single texture evaluation per bounce
        vec3  N      = hit.normal;
        vec3  albedo = mat.albedo.rgb;
        float alpha  = 1.0;
        float rough  = mat.roughness;
        trace_textures(mat, N, hit, albedo, alpha, rough);

        // Alpha test
        if (alpha < 1.0 && rand(seed) > alpha)
        {
            ray.origin = hit.pos + ray.dir * adaptive_bias;
            bounce--;
            continue;
        }

        s_shade_result res = shade_hit(
            ray, hit, N, albedo, rough,
            throughput, prev_specular, prev_bsdf_pdf, prev_origin, seed
        );

        radiance += res.direct_radiance;

        if (res.terminate)
            break;

        throughput = res.new_throughput;

        if (max(throughput.r, max(throughput.g, throughput.b)) < 0.001)
            break;

        prev_bsdf_pdf = res.new_bsdf_pdf;
        prev_specular = res.is_specular;
        prev_origin   = ray.origin;

        ray.origin  = hit.pos + N * adaptive_bias;
        ray.dir     = res.new_dir;
        ray.inv_dir = 1.0 / res.new_dir;

        if (bounce >= 1)
        {
            float p = clamp(max(throughput.r, max(throughput.g, throughput.b)), 0.05, 0.95);
            if (rand(seed) > p) break;
            throughput /= p;
        }
    }
    return radiance;
}

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy) + ivec2(u_tile_offset);
    if (pixel.x >= int(u_resolution.x) ||
        pixel.y >= int(u_resolution.y))
        return;

    uint seed = wang_hash(uint(pixel.x) ^ wang_hash(uint(pixel.y) ^ wang_hash(uint(u_frame_index))));

    vec2 jitter = vec2(rand(seed), rand(seed));

    vec2 uv = (vec2(pixel) + jitter) / u_resolution * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    float half_fov = tan(u_cam_fov * 0.5);

    vec3 ray_dir = normalize(
        u_cam_forward +
        uv.x * half_fov * u_cam_right +
        uv.y * half_fov * u_cam_up);

    s_ray ray;
    ray.origin  = u_cam_pos;
    ray.dir     = ray_dir;
    ray.inv_dir = 1.0 / ray_dir;

    vec3 new_sample = trace_path(ray, seed);

    vec4  prev         = imageLoad(u_output, pixel);
    float sample_count = float(u_frame_index);

    if (u_reset_samples == 1u)
    {
        prev         = vec4(0.0);
        sample_count = 0.0;
    }

    prev.rgb += new_sample;
    sample_count += 1.0;

    imageStore(u_output, pixel, vec4(prev.rgb, sample_count));
}
