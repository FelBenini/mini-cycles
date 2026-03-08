bool scene_intersect(s_ray ray_world, out s_hit hit)
{
    hit.t  = 1e30;
    bool found = false;

    uint stack[96];
    uint ptr = 0;
    stack[ptr++] = 0;

    while (ptr > 0)
    {
        uint        idx  = stack[--ptr];
        s_tlas_node node = tlas_nodes[idx];

        float tnear;
        if (!intersect_aabb(ray_world,
                            node.bbox_min.xyz,
                            node.bbox_max.xyz,
                            hit.t,
                            tnear))
            continue;

        // Leaf — intersect the mesh in its local space
        if (node.left_child == 0 && node.right_child == 0)
        {
            uint mesh_idx  = node.mesh_index;
            vec3 mesh_pos  = meshes[mesh_idx].position.xyz;
            vec3 mesh_fwd  = meshes[mesh_idx].direction.xyz;

            mat3 R     = mat_from_dir(mesh_fwd);
            mat3 R_inv = transpose(R);

            s_ray ray;
            ray.origin  = R_inv * (ray_world.origin - mesh_pos);
            ray.dir     = R_inv * ray_world.dir;
            ray.inv_dir = 1.0 / ray.dir;

            float t_before = hit.t;
            blas_intersect(ray, mesh_idx, hit);

            if (hit.t < t_before)
            {
                hit.normal     = normalize(R * hit.normal);
                hit.geo_normal = normalize(R * hit.geo_normal);
                found = true;
            }

            continue;
        }

        // ---- Ordered child traversal ----
        float t_left  = 1e30;
        float t_right = 1e30;

        bool hit_left  = (node.left_child  != 0) && intersect_aabb(ray_world,
                            tlas_nodes[node.left_child].bbox_min.xyz,
                            tlas_nodes[node.left_child].bbox_max.xyz,
                            hit.t, t_left);

        bool hit_right = (node.right_child != 0) && intersect_aabb(ray_world,
                            tlas_nodes[node.right_child].bbox_min.xyz,
                            tlas_nodes[node.right_child].bbox_max.xyz,
                            hit.t, t_right);

        if (hit_left && hit_right)
        {
            if (t_left < t_right) {
                stack[ptr++] = node.right_child;
                stack[ptr++] = node.left_child;
            } else {
                stack[ptr++] = node.left_child;
                stack[ptr++] = node.right_child;
            }
        }
        else if (hit_left)  stack[ptr++] = node.left_child;
        else if (hit_right) stack[ptr++] = node.right_child;
    }

    if (found)
        hit.pos = ray_world.origin + ray_world.dir * hit.t;

    return found;
}

bool scene_intersect_shadow(s_ray ray_world, float max_t)
{
    uint stack[96];
    uint ptr = 0;
    stack[ptr++] = 0;

    while (ptr > 0)
    {
        uint        idx  = stack[--ptr];
        s_tlas_node node = tlas_nodes[idx];

        float tnear;
        if (!intersect_aabb(ray_world, node.bbox_min.xyz,
                            node.bbox_max.xyz, max_t, tnear))
            continue;

        if (node.left_child == 0 && node.right_child == 0)
        {
            uint mesh_idx = node.mesh_index;
            vec3 mesh_pos = meshes[mesh_idx].position.xyz;
            mat3 R_inv    = transpose(mat_from_dir(meshes[mesh_idx].direction.xyz));

            s_ray ray;
            ray.origin  = R_inv * (ray_world.origin - mesh_pos);
            ray.dir     = R_inv * ray_world.dir;
            ray.inv_dir = 1.0 / ray.dir;

            uint bstack[96];
            uint bptr = 0;
            bstack[bptr++] = meshes[mesh_idx].bvh_root;

            while (bptr > 0)
            {
                s_bvh_node bnode = bvh_nodes[bstack[--bptr]];

                if (bnode.tri_count > 0)
                {
                    uint base = meshes[mesh_idx].tri_offset + bnode.tri_offset;
                    for (uint i = 0; i < bnode.tri_count; i++)
                    {
                        float t; vec3 bary;
                        if (intersect_triangle(ray, triangles[base + i], t, bary)
                            && t < max_t)
                            return true;
                    }
                    continue;
                }

                float tl, tr;
                bool hl = (bnode.left_child  != 0) && intersect_aabb(ray,
                    bvh_nodes[bnode.left_child].bbox_min.xyz,
                    bvh_nodes[bnode.left_child].bbox_max.xyz, max_t, tl);
                bool hr = (bnode.right_child != 0) && intersect_aabb(ray,
                    bvh_nodes[bnode.right_child].bbox_min.xyz,
                    bvh_nodes[bnode.right_child].bbox_max.xyz, max_t, tr);
                if (hl)
                  bstack[bptr++] = bnode.left_child;
                if (hr)
                  bstack[bptr++] = bnode.right_child;
            }
            continue;
        }

        float tl, tr;
        bool hl = (node.left_child  != 0) && intersect_aabb(ray_world,
            tlas_nodes[node.left_child].bbox_min.xyz,
            tlas_nodes[node.left_child].bbox_max.xyz, max_t, tl);
        bool hr = (node.right_child != 0) && intersect_aabb(ray_world,
            tlas_nodes[node.right_child].bbox_min.xyz,
            tlas_nodes[node.right_child].bbox_max.xyz, max_t, tr);
        if (hl)
          stack[ptr++] = node.left_child;
        if (hr)
          stack[ptr++] = node.right_child;
    }
    return false;
}

