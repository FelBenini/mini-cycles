#ifndef SCENE_H
# define SCENE_H

# include "../glad/include/glad/glad.h"
# include "bvh.h"
# include "camera.h"
# include "material.h"
# include "mesh.h"
# include "rt_math.h"
# include <stdint.h>


typedef struct s_image_metadata {
	int					width;
	int					height;
	int					channels;
}						t_image_data;

typedef struct s_image_ssbo{
	GLuint			ssbo;
	GLuint			ssbo_pixels;
	t_image_data		*metadata;
	int			count;
	size_t			total_bytes;
}				t_image_ssbo;

// Mirror of s_mesh_descriptor in the compute shader (std430 aligned)
typedef struct s_mesh_descriptor
{
	t_vec4				position; // xyz = world position, w = bounding radius
	t_vec4				direction;
	uint32_t			tri_offset; // start index into the global triangle array
	uint32_t			tri_count;
	uint32_t			smooth;
	uint32_t			bvh_root;
	uint32_t			material;
	uint32_t			pad[3];
}						t_mesh_descriptor;

typedef struct s_scene
{
	// CPU side
	t_mesh				*meshes;
	t_mesh_descriptor	*descriptors;
	t_material			*materials;
	t_triangle			*triangles;
	t_bvh				*bvhs;
	t_tlas				tlas;
	t_vec4				ambient;
	int					sky_tex;
	uint32_t			mesh_count;
	uint32_t			mesh_capacity;
	uint32_t			material_count;
	uint32_t			material_capacity;
	uint32_t			triangle_count;
	t_camera			camera;
	t_image_ssbo		*images;

	GLuint				ssbo_triangles;
	GLuint				ssbo_normals;
	GLuint				ssbo_meshes;
	GLuint				ssbo_bvh_nodes;
	GLuint				ssbo_tlas_nodes;
	GLuint				ssbo_materials;
	int					gpu_dirty;
	int					desc_dirty;
	int					bvh_dirty;
	int					tlas_dirty;
	int					material_dirty;
}						t_scene;

t_scene					scene_create(uint32_t initial_capacity);
void					scene_destroy(t_scene *scene);

uint32_t				scene_add_mesh(t_scene *scene, t_mesh mesh,
							uint32_t material_index);

uint32_t				scene_add_material(t_scene *scene, t_material material);

void					scene_upload_triangles(t_scene *scene);

void					scene_upload_descriptors(t_scene *scene);

void					scene_upload_materials(t_scene *scene);

void					scene_upload_bvh_nodes(t_scene *scene);

void					scene_upload_tlas_nodes(t_scene *scene);

void					scene_rebuild_tlas(t_scene *scene);

void					scene_move_mesh(t_scene *scene, uint32_t index,
							t_vec4 position);

int						scene_load_image(t_scene *scene, const char *path);
void					scene_upload_images(t_scene *scene);
void					scene_destroy_images(t_scene *scene);

#endif
