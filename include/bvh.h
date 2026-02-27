#ifndef BVH_H
#define BVH_H

#include <stdint.h>
#include "rt_math.h"
#include "triangle.h"

// Forward declaration
typedef struct s_mesh_descriptor t_mesh_descriptor;

// BVH node for GPU (32 bytes per node, std430 aligned)
typedef struct s_bvh_node {
	t_vec4		bbox_min;      // xyz = min corner, w = unused
	t_vec4		bbox_max;      // xyz = max corner, w = unused
	uint32_t	left_child;    // index of left child (0 = leaf)
	uint32_t	right_child;   // index of right child (0 = leaf)
	uint32_t	tri_offset;    // triangle offset (leaves only)
	uint32_t	tri_count;     // triangle count (leaves only; 0 = internal)
}	t_bvh_node;

// BVH structure for per-mesh acceleration
typedef struct s_bvh {
	t_bvh_node	*nodes;
	uint32_t	*indices;   // reordered triangle indices
	uint32_t	node_count;
	uint32_t	index_count;
}	t_bvh;

typedef struct s_builder_ctx
{
	t_bvh_node	*nodes;
	uint32_t	*node_idx;
	t_triangle	*triangles;
	uint32_t	*indices;
	uint32_t	max_leaf_size;
}	t_builder_ctx;

// Compute a bounding box of a group of triangles
void	compute_bbox(t_triangle *triangles, uint32_t *indices, uint32_t start,
						uint32_t count, t_vec4 *out_min, t_vec4 *out_max);

uint32_t	bvh_build_recursive(t_builder_ctx *ctx, uint32_t start, uint32_t count);

// Build a BVH for a mesh
t_bvh bvh_build(t_triangle *triangles, uint32_t tri_count);

// Destroy a BVH
void bvh_destroy(t_bvh *bvh);

typedef struct s_tlas_node {
	t_vec4		bbox_min;
	t_vec4		bbox_max;
	uint32_t	left_child;
	uint32_t	right_child;
	uint32_t	mesh_index;
	uint32_t	_padding;
}	t_tlas_node;

typedef struct s_tlas {
	t_tlas_node	*nodes;
	uint32_t	node_count;
}	t_tlas;

typedef struct s_tlas_builder_ctx
{
	t_tlas_node			*nodes;
	uint32_t			*node_idx;
	t_mesh_descriptor	*descriptors;
	uint32_t			mesh_count;
}	t_tlas_builder_ctx;

// Build a TLAS for the scene
t_tlas tlas_build(t_mesh_descriptor *descriptors, uint32_t mesh_count);

uint32_t	tlas_build_recursive(t_tlas_builder_ctx *ctx, uint32_t *indices,
									uint32_t start, uint32_t count);

// Destroy a TLAS
void tlas_destroy(t_tlas *tlas);

#endif
