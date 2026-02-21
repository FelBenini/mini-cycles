#include "bvh.h"
#include <stdlib.h>

void	bvh_destroy(t_bvh *bvh)
{
	if (bvh->nodes)
	{
		free(bvh->nodes);
		bvh->nodes = NULL;
	}
	if (bvh->indices)
	{
		free(bvh->indices);
		bvh->indices = NULL;
	}
	bvh->node_count = 0;
	bvh->index_count = 0;
}
