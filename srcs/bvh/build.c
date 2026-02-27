#include "bvh.h"
#include <stdint.h>
#include <stdlib.h>

t_bvh	bvh_build(t_triangle *triangles, uint32_t tri_count)
{
	t_bvh			bvh = {0};
	uint32_t		max_nodes;
	uint32_t		i;
	t_builder_ctx	ctx;

	if (tri_count == 0)
		return (bvh);
	max_nodes = 2 * tri_count;
	bvh.nodes = malloc(max_nodes * sizeof(t_bvh_node));
	bvh.indices = malloc(tri_count * sizeof(uint32_t));
	i = 0;
	while (i < tri_count)
	{
		bvh.indices[i] = i;
		i++;
	}
	ctx.nodes = bvh.nodes;
	ctx.node_idx = &bvh.node_count;
	ctx.triangles = triangles;
	ctx.indices = bvh.indices;
	ctx.max_leaf_size = 4;
	bvh_build_recursive(&ctx, 0, tri_count);
	bvh.index_count = tri_count;
	return (bvh);
}

t_tlas	tlas_build(t_mesh_descriptor *descriptors, uint32_t mesh_count)
{
	t_tlas				tlas = {0};
	uint32_t			max_nodes;
	uint32_t			*indices;
	uint32_t			i;
	t_tlas_builder_ctx	ctx;

	if (mesh_count == 0)
		return (tlas);
	max_nodes = 2 * mesh_count - 1;
	if (max_nodes < 1)
		max_nodes = 1;
	tlas.nodes = malloc(max_nodes * sizeof(t_tlas_node));
	indices = malloc(mesh_count * sizeof(uint32_t));
	if (!tlas.nodes || !indices)
	{
		free(tlas.nodes);
		free(indices);
		return (tlas);
	}
	i = 0;
	while (i < mesh_count)
	{
		indices[i] = i;
		i++;
	}
	ctx.nodes = tlas.nodes;
	ctx.descriptors = descriptors;
	ctx.mesh_count = mesh_count;
	ctx.node_idx = &tlas.node_count;
	tlas_build_recursive(&ctx, indices, 0, mesh_count);
	free(indices);
	return (tlas);
}
