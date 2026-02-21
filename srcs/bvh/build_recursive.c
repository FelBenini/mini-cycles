#include <stdint.h>
#include "bvh.h"

// find split position by computing spatial median
static float	get_split_pos(t_bvh_node *node, int *axis)
{
	float	split_pos;
	float	dx;
	float	dy;
	float	dz;
	float	max_extent;

	split_pos = 0.0f;
	dx = node->bbox_max.x - node->bbox_min.x;
	dy = node->bbox_max.y - node->bbox_min.y;
	dz = node->bbox_max.z - node->bbox_min.z;
	*axis = 0;
	max_extent = dx;
	if (dy > max_extent)
	{
		*axis = 1;
		max_extent = dy;
	}
	if (dz > max_extent)
	{
		*axis = 2;
		max_extent = dz;
	}
    if (*axis == 0)
		split_pos = node->bbox_min.x + (node->bbox_max.x - node->bbox_min.x) * 0.5f;
    else if (*axis == 1)
		split_pos = node->bbox_min.y + (node->bbox_max.y - node->bbox_min.y) * 0.5f;
    else
		split_pos = node->bbox_min.z + (node->bbox_max.z - node->bbox_min.z) * 0.5f;
	return (split_pos);
}

static t_vec4	get_triangle_center(t_triangle tri)
{
	t_vec4	center;

	center.x = (tri.v0.x + tri.v1.x + tri.v2.x) / 3.0f;
	center.y = (tri.v0.y + tri.v1.y + tri.v2.y) / 3.0f;
	center.z = (tri.v0.z + tri.v1.z + tri.v2.z) / 3.0f;
	center.w = 0.0f;
	return (center);
}

uint32_t	bvh_build_recursive(t_builder_ctx *ctx, uint32_t start, uint32_t count)
{
	uint32_t	curr_node_idx;
	t_bvh_node	*node;
	float		split_pos;
	uint32_t	left;
	uint32_t	right;
	t_vec4		center;
	int			axis;
	float		coord;
	uint32_t	left_count;
	uint32_t	right_count;

	curr_node_idx = (*ctx->node_idx)++;
	node = &ctx->nodes[curr_node_idx];
	compute_bbox(ctx->triangles, ctx->indices, start, count, &node->bbox_min, &node->bbox_max);
	if (count <= ctx->max_leaf_size)
	{
		node->left_child = 0;
		node->right_child = 0;
		node->tri_offset = start;
		node->tri_count = count;
		return (curr_node_idx);
	}
	split_pos = get_split_pos(node, &axis);
	left = start;
	right = start + count - 1;
	while (left <= right)
	{
		center = get_triangle_center(ctx->triangles[ctx->indices[left]]);
		if (axis == 0)
			coord = center.x;
		if (axis == 1)
			coord = center.y;
		if (axis == 2)
			coord = center.z;
		if (coord < split_pos)
			left++;
		else
		{
			uint32_t	tmp = ctx->indices[left];
			ctx->indices[left] = ctx->indices[right];
			ctx->indices[right] = tmp;
			if (right == 0)
				break ;
			right--;
		}
	}
	left_count = left - start;
	right_count = count - left_count;
	if (left_count == 0 || right_count == 0)
	{
		left_count = count / 2;
		right_count = count - left_count;
	}
	node->left_child = bvh_build_recursive(ctx, start, left_count);
	node->right_child = bvh_build_recursive(ctx, start + left_count, right_count);
	node->tri_offset = 0;
	node->tri_count = 0;
	return (curr_node_idx);
}
