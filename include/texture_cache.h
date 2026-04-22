#ifndef TEXTURE_CACHE_H
# define TEXTURE_CACHE_H

# include <stdint.h>

/*
** Hash map for texture caching using chaining for collision handling.
** Key: image path/name (string)
** Value: image buffer index (int)
*/

typedef struct s_cache_entry
{
	char					*path;
	int						image_index;
	struct s_cache_entry	*next;
}						t_cache_entry;

typedef struct s_texture_cache
{
	t_cache_entry	**buckets;
	uint32_t		capacity;
	uint32_t		size;
}					t_texture_cache;

/* Create a new texture cache with given capacity */
t_texture_cache		*texture_cache_create(uint32_t capacity);

/* Lookup a texture by path. Returns image index or -1 if not found */
int					texture_cache_lookup(t_texture_cache *cache, const char *path);

/* Insert a new entry (path -> image_index). Returns 0 on success, -1 on failure */
int					texture_cache_insert(t_texture_cache *cache, const char *path,
						int image_index);

/* Destroy and free all resources */
void				texture_cache_destroy(t_texture_cache *cache);

#endif
