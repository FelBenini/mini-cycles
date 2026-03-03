#include <stdint.h>
#include "bvh.h"

static t_vec4	get_triangle_center(t_triangle tri)
{
	t_vec4	center;

	center.x = (tri.v0.x + tri.v1.x + tri.v2.x) / 3.0f;
	center.y = (tri.v0.y + tri.v1.y + tri.v2.y) / 3.0f;
	center.z = (tri.v0.z + tri.v1.z + tri.v2.z) / 3.0f;
	center.w = 0.0f;
	return (center);
}

static float	triangle_sah_surface_area(t_vec4 mn, t_vec4 mx)
{
	float	ex;
	float	ey;
	float	ez;

	ex = mx.x - mn.x;
	ey = mx.y - mn.y;
	ez = mx.z - mn.z;
	return (2.0f * (ex * ey + ey * ez + ez * ex));
}

static void	expand_bbox(t_triangle *tri, t_vec4 *mn, t_vec4 *mx)
{
	float	minx;
	float	miny;
	float	minz;
	float	maxx;
	float	maxy;
	float	maxz;

	minx = tri->v0.x < tri->v1.x ? tri->v0.x : tri->v1.x;
	minx = minx      < tri->v2.x ? minx       : tri->v2.x;
	miny = tri->v0.y < tri->v1.y ? tri->v0.y : tri->v1.y;
	miny = miny      < tri->v2.y ? miny       : tri->v2.y;
	minz = tri->v0.z < tri->v1.z ? tri->v0.z : tri->v1.z;
	minz = minz      < tri->v2.z ? minz       : tri->v2.z;

	maxx = tri->v0.x > tri->v1.x ? tri->v0.x : tri->v1.x;
	maxx = maxx      > tri->v2.x ? maxx       : tri->v2.x;
	maxy = tri->v0.y > tri->v1.y ? tri->v0.y : tri->v1.y;
	maxy = maxy      > tri->v2.y ? maxy       : tri->v2.y;
	maxz = tri->v0.z > tri->v1.z ? tri->v0.z : tri->v1.z;
	maxz = maxz      > tri->v2.z ? maxz       : tri->v2.z;

	if (minx < mn->x) mn->x = minx;
	if (miny < mn->y) mn->y = miny;
	if (minz < mn->z) mn->z = minz;
	if (maxx > mx->x) mx->x = maxx;
	if (maxy > mx->y) mx->y = maxy;
	if (maxz > mx->z) mx->z = maxz;
}

static float	get_split_pos_sah(t_builder_ctx *ctx,
								t_bvh_node *node,
								uint32_t start, uint32_t count,
								int *out_axis)
{
	const int	NUM_BUCKETS = 12;
	float		best_cost;
	float		best_pos;
	int			best_axis;
	int			axis;
	int			b;
	uint32_t	i;
	float		parent_sa;
	t_vec4		bmin[12];
	t_vec4		bmax[12];
	uint32_t	bcnt[12];
	t_vec4		suffix_min[12];
	t_vec4		suffix_max[12];

	parent_sa = triangle_sah_surface_area(node->bbox_min, node->bbox_max);
	best_cost = 1e30f;
	best_pos  = 0.0f;
	best_axis = 0;
	axis = 0;
	while (axis < 3)
	{
		float ax_min = (&node->bbox_min.x)[axis];
		float ax_max = (&node->bbox_max.x)[axis];
		float ax_ext = ax_max - ax_min;
		if (ax_ext < 1e-6f)
		{
			axis++;
			continue;
		}
		float inv_ext = (float)NUM_BUCKETS / ax_ext;
		b = 0;
		while (b < NUM_BUCKETS)
		{
			bmin[b].x = bmin[b].y = bmin[b].z =  1e30f; bmin[b].w = 0.0f;
			bmax[b].x = bmax[b].y = bmax[b].z = -1e30f; bmax[b].w = 0.0f;
			bcnt[b] = 0;
			b++;
		}
		i = start;
		while (i < start + count)
		{
			t_triangle *tri = &ctx->triangles[ctx->indices[i]];
			t_vec4 c = get_triangle_center(*tri);
			int bi = (int)(((&c.x)[axis] - ax_min) * inv_ext);
			if (bi < 0)          bi = 0;
			if (bi >= NUM_BUCKETS) bi = NUM_BUCKETS - 1;
			expand_bbox(tri, &bmin[bi], &bmax[bi]);
			bcnt[bi]++;
			i++;
		}
		suffix_min[NUM_BUCKETS - 1] = bmin[NUM_BUCKETS - 1];
		suffix_max[NUM_BUCKETS - 1] = bmax[NUM_BUCKETS - 1];
		b = NUM_BUCKETS - 2;
		while (b >= 0)
		{
			suffix_min[b].x = bmin[b].x < suffix_min[b+1].x ? bmin[b].x : suffix_min[b+1].x;
			suffix_min[b].y = bmin[b].y < suffix_min[b+1].y ? bmin[b].y : suffix_min[b+1].y;
			suffix_min[b].z = bmin[b].z < suffix_min[b+1].z ? bmin[b].z : suffix_min[b+1].z;
			suffix_min[b].w = 0.0f;
			suffix_max[b].x = bmax[b].x > suffix_max[b+1].x ? bmax[b].x : suffix_max[b+1].x;
			suffix_max[b].y = bmax[b].y > suffix_max[b+1].y ? bmax[b].y : suffix_max[b+1].y;
			suffix_max[b].z = bmax[b].z > suffix_max[b+1].z ? bmax[b].z : suffix_max[b+1].z;
			suffix_max[b].w = 0.0f;
			b--;
		}
		t_vec4 pmin = bmin[0];
		t_vec4 pmax = bmax[0];
		uint32_t left_count = bcnt[0];
		b = 1;
		while (b < NUM_BUCKETS)
		{
			uint32_t right_count = 0;
			int rb = b;
			while (rb < NUM_BUCKETS) { right_count += bcnt[rb]; rb++; }

			if (left_count > 0 && right_count > 0)
			{
				float sa_left  = triangle_sah_surface_area(pmin, pmax);
				float sa_right = triangle_sah_surface_area(suffix_min[b], suffix_max[b]);
				float cost = (sa_left * (float)left_count
				            + sa_right * (float)right_count) / parent_sa;

				if (cost < best_cost)
				{
					best_cost = cost;
					best_axis = axis;
					best_pos  = ax_min + (float)b / (float)NUM_BUCKETS * ax_ext;
				}
			}
			if (bmin[b].x < pmin.x) pmin.x = bmin[b].x;
			if (bmin[b].y < pmin.y) pmin.y = bmin[b].y;
			if (bmin[b].z < pmin.z) pmin.z = bmin[b].z;
			if (bmax[b].x > pmax.x) pmax.x = bmax[b].x;
			if (bmax[b].y > pmax.y) pmax.y = bmax[b].y;
			if (bmax[b].z > pmax.z) pmax.z = bmax[b].z;
			left_count += bcnt[b];
			b++;
		}
		axis++;
	}

	*out_axis = best_axis;
	return (best_pos);
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
	split_pos = get_split_pos_sah(ctx, node, start, count, &axis);
	left = start;
	right = start + count - 1;
	while (left <= right)
	{
		coord = 0;
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
