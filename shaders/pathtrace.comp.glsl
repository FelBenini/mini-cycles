vec3 sample_hemisphere(vec3 N, inout uint seed)
{
    float r1 = rand(seed);
    float r2 = rand(seed);

    float phi       = 2.0 * 3.14159265 * r1;
    float cos_theta = sqrt(1.0 - r2);
    float sin_theta = sqrt(r2);

    vec3 T = normalize(abs(N.x) > 0.1
        ? cross(vec3(0,1,0), N)
        : cross(vec3(1,0,0), N));
    vec3 B = cross(N, T);

    return normalize(
        cos(phi) * sin_theta * T +
        sin(phi) * sin_theta * B +
        cos_theta * N);
}

vec3 trace_path(s_ray ray, inout uint seed)
{
    vec3 throughput = vec3(1.0);
    vec3 radiance   = vec3(0.0);

    const int MAX_BOUNCES = 4;


    for (int bounce = 0; bounce < MAX_BOUNCES; bounce++)
    {
        s_hit hit;

        if (!scene_intersect(ray, hit))
        {
            radiance += throughput * sky_color(ray);
            break;
        }

        s_mesh_descriptor mesh = meshes[hit.mesh_index];
        s_material        mat  = materials[mesh.material];

        vec3 N        = hit.normal;
        vec3 albedo   = mat.albedo.rgb;
        vec3 emission = mat.emission.rgb;

        radiance += throughput * emission;

        if (length(emission) > 0.0)
            break;

        float adaptive_bias = max(1e-4, hit.t * 1e-4);

        float rough       = clamp(mat.roughness, 0.001, 1.0);
        vec3  R           = reflect(ray.dir, N);
        vec3  glossy_dir  = normalize(R + rough * sample_hemisphere(N, seed));
        vec3  diffuse_dir = sample_hemisphere(N, seed);
        vec3  new_dir     = normalize(mix(glossy_dir, diffuse_dir, rough));

        ray.origin  = hit.pos + hit.geo_normal * adaptive_bias;
        ray.dir     = new_dir;
        ray.inv_dir = 1.0 / new_dir;

        vec3 specular_color = mix(vec3(1.0), albedo, mat.metallic);
        vec3 diffuse_color  = albedo * (1.0 - mat.metallic);
        throughput *= mix(specular_color, diffuse_color, rough);

        float p = max(throughput.r, max(throughput.g, throughput.b));
        if (bounce > 0)
        {
            if (rand(seed) > p) break;
            throughput /= p;
        }
    }

    return radiance;
}

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= int(u_resolution.x) ||
        pixel.y >= int(u_resolution.y))
        return;

    uint seed = wang_hash(uint(pixel.x) ^ wang_hash(uint(pixel.y) ^ wang_hash(u_frame_index)));

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

    vec3 accum = (prev.rgb * sample_count + new_sample)
               / (sample_count + 1.0);

    imageStore(u_output, pixel, vec4(accum, 1.0));
}
