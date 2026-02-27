#ifndef SCENE_H
# define SCENE_H

# include <stdint.h>
# include "../glad/include/glad/glad.h"
# include "mesh.h"
# include "rt_math.h"
# include "bvh.h"

// Mirror of s_mesh_descriptor in the compute shader (std430 aligned)
typedef struct s_mesh_descriptor {
    t_vec4   position;    // xyz = world position, w = bounding radius
    uint32_t tri_offset;  // start index into the global triangle array
    uint32_t tri_count;
	uint32_t smooth;
	uint32_t bvh_root;
} t_mesh_descriptor;

typedef struct s_scene {
    // CPU side
    t_mesh            *meshes;        // array of meshes
    t_mesh_descriptor *descriptors;   // parallel array — one per mesh
    t_triangle        *triangles;     // flat global triangle array
    t_bvh             *bvhs;          // BVH per mesh
    t_tlas             tlas;          // TLAS for scene (top-level acceleration structure)
    uint32_t           mesh_count;
    uint32_t           mesh_capacity;
    uint32_t           triangle_count;

    // GPU side
    GLuint             ssbo_triangles; // binding = 1
    GLuint             ssbo_normals;   // binding = 2
    GLuint             ssbo_meshes;    // binding = 3
    GLuint             ssbo_bvh_nodes; // binding = 4
    GLuint             ssbo_tlas_nodes; // binding = 5
    int                gpu_dirty;      // triangles need re-upload
    int                desc_dirty;     // descriptors need re-upload
    int                bvh_dirty;      // BVH nodes need re-upload
    int                tlas_dirty;     // TLAS nodes need re-upload
} t_scene;

t_scene  scene_create(uint32_t initial_capacity);
void     scene_destroy(t_scene *scene);

uint32_t scene_add_mesh(t_scene *scene, t_mesh mesh);

void     scene_upload_triangles(t_scene *scene);

void     scene_upload_descriptors(t_scene *scene);

void     scene_upload_bvh_nodes(t_scene *scene);

void     scene_upload_tlas_nodes(t_scene *scene);

void     scene_rebuild_tlas(t_scene *scene);

void     scene_move_mesh(t_scene *scene, uint32_t index, t_vec4 position);

#endif
