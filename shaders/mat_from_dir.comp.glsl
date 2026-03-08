mat3 mat_from_dir(vec3 up)
{
    float len = length(up);
    if (len < 1e-6)
        return mat3(1.0);

    up = up / len;
    vec3 world_fwd = abs(up.z) < 0.999 ? vec3(0.0, 0.0, 1.0)
                                        : vec3(0.0, 1.0, 0.0);
    vec3 right   = normalize(cross(world_fwd, up));
    vec3 forward = cross(up, right);
    return mat3(right, up, forward);
}
