mat3 mat_from_dir(vec3 forward)
{
    float len = length(forward);
    if (len < 1e-6)
        return mat3(1.0);

    forward = normalize(forward);

    vec3 world_up = abs(forward.y) < 0.999
        ? vec3(0.0, 1.0, 0.0)
        : vec3(1.0, 0.0, 0.0);

    vec3 right = normalize(cross(world_up, forward));
    vec3 up = cross(forward, right);

    return mat3(right, up, forward);
}
