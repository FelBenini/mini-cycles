#include "bvh.h"
#include "scene.h"
#include <stdint.h>

static void	compute_mesh_bbox(t_mesh_descriptor *descriptor,
								t_vec4 *out_min, t_vec4 *out_max)
{
	t_vec3	pos;
	float	radius;

	pos.x = descriptor->position.x;
	pos.y = descriptor->position.y;
	pos.z = descriptor->position.z;
	radius = descriptor->position.w;

	out_min->x = pos.x - radius;
	out_min->y = pos.y - radius;
	out_min->z = pos.z - radius;
	out_min->w = 0.0f;

	out_max->x = pos.x + radius;
	out_max->y = pos.y + radius;
	out_max->z = pos.z + radius;
	out_max->w = 0.0f;
}

static void	compute_meshes_bbox(t_mesh_descriptor *descriptors, uint32_t *indices,
								uint32_t start, uint32_t count,
								t_vec4 *out_min, t_vec4 *out_max)
{
	uint32_t	i;
	t_vec4		mesh_min;
	t_vec4		mesh_max;

	if (count == 0)
		return ;

	compute_mesh_bbox(&descriptors[indices[start]], out_min, out_max);
	i = start + 1;
	while (i < start + count)
	{
		compute_mesh_bbox(&descriptors[indices[i]], &mesh_min, &mesh_max);

		if (mesh_min.x < out_min->x) out_min->x = mesh_min.x;
		if (mesh_min.y < out_min->y) out_min->y = mesh_min.y;
		if (mesh_min.z < out_min->z) out_min->z = mesh_min.z;

		if (mesh_max.x > out_max->x) out_max->x = mesh_max.x;
		if (mesh_max.y > out_max->y) out_max->y = mesh_max.y;
		if (mesh_max.z > out_max->z) out_max->z = mesh_max.z;
		i++;
	}
}

static uint32_t	find_split_index(t_mesh_descriptor *descriptors, uint32_t *indices,
								uint32_t start, uint32_t count,
								t_vec4 bbox_min, t_vec4 bbox_max)
{
	float				dx;
	float				dy;
	float				dz;
	uint32_t			axis;
	float				split_pos;
	uint32_t			lo;
	uint32_t			hi;
	uint32_t			temp;
	float				pos_component;
	t_mesh_descriptor	*desc;

	dx = bbox_max.x - bbox_min.x;
	dy = bbox_max.y - bbox_min.y;
	dz = bbox_max.z - bbox_min.z;
	axis = 0;
	if (dy > dx)
		axis = 1;
	if (dz > (axis == 0 ? dx : dy))
		axis = 2;
	if (axis == 0)
		split_pos = bbox_min.x + dx * 0.5f;
	else if (axis == 1)
		split_pos = bbox_min.y + dy * 0.5f;
	else
		split_pos = bbox_min.z + dz * 0.5f;
	lo = start;
	hi = start + count - 1;
	while (lo <= hi)
	{
		desc = &descriptors[indices[lo]];
		if (axis == 0)
			pos_component = desc->position.x;
		else if (axis == 1)
			pos_component = desc->position.y;
		else
			pos_component = desc->position.z;

		if (pos_component < split_pos)
		{
			lo++;
		}
		else
		{
			temp      = indices[lo];
			indices[lo] = indices[hi];
			indices[hi] = temp;
			if (hi == 0)
				break ;
			hi--;
		}
	}
	if (lo == start)
		lo = start + 1;
	else if (lo == start + count)
		lo = start + count - 1;
	return (lo);
}

static uint32_t	filter_invalid_indices(uint32_t *indices,
										uint32_t start, uint32_t count,
										uint32_t max_descriptors)
{
	uint32_t	read;
	uint32_t	write;

	write = start;
	read  = start;
	while (read < start + count)
	{
		if (indices[read] < max_descriptors)
		{
			indices[write] = indices[read];
			write++;
		}
		read++;
	}
	return (write - start);
}

uint32_t	tlas_build_recursive(t_tlas_builder_ctx *ctx, uint32_t *indices,
								uint32_t start, uint32_t count)
{
	uint32_t	node_idx;
	t_tlas_node	*node;
	t_vec4		bbox_min;
	t_vec4		bbox_max;
	uint32_t	split_idx;
	uint32_t	left_count;
	uint32_t	right_count;

	count = filter_invalid_indices(indices, start, count, ctx->mesh_count);
	if (count == 0)
		return (0);
	node_idx = *ctx->node_idx;
	if (node_idx >= 2 * ctx->mesh_count - 1)
		return (0);
	(*ctx->node_idx)++;
	node = &ctx->nodes[node_idx];
	compute_meshes_bbox(ctx->descriptors, indices, start, count, &bbox_min, &bbox_max);
	node->bbox_min = bbox_min;
	node->bbox_max = bbox_max;
	if (count == 1)
	{
		node->left_child  = 0;
		node->right_child = 0;
		node->mesh_index  = indices[start];
		node->_padding    = 0;
		return (node_idx);
	}
	split_idx   = find_split_index(ctx->descriptors, indices, start, count,
									bbox_min, bbox_max);
	left_count  = split_idx - start;
	right_count = start + count - split_idx;
	node->left_child  = tlas_build_recursive(ctx, indices, start,      left_count);
	node->right_child = tlas_build_recursive(ctx, indices, split_idx,  right_count);
	node->mesh_index  = 0;
	node->_padding    = 0;
	return (node_idx);
}
