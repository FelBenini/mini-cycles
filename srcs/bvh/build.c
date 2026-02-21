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
