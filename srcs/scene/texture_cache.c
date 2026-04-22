#include "texture_cache.h"
#include <stdlib.h>
#include <string.h>

/*
** DJB2 hash function - simple and effective for string hashing
** Reference: http://www.cse.yorku.ca/~oz/hash.html
*/
static uint32_t	hash_djb2(const char *str)
{
	uint32_t	hash;
	int			c;

	hash = 5381;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	return (hash);
}

t_texture_cache	*texture_cache_create(uint32_t capacity)
{
	t_texture_cache	*cache;

	if (capacity == 0)
		return (NULL);
	cache = malloc(sizeof(t_texture_cache));
	if (!cache)
		return (NULL);
	cache->buckets = calloc(capacity, sizeof(t_cache_entry *));
	if (!cache->buckets)
	{
		free(cache);
		return (NULL);
	}
	cache->capacity = capacity;
	cache->size = 0;
	return (cache);
}

int	texture_cache_lookup(t_texture_cache *cache, const char *path)
{
	uint32_t		hash;
	uint32_t		index;
	t_cache_entry	*entry;

	if (!cache || !path)
		return (-1);
	hash = hash_djb2(path);
	index = hash % cache->capacity;
	entry = cache->buckets[index];
	while (entry)
	{
		if (strcmp(entry->path, path) == 0)
			return (entry->image_index);
		entry = entry->next;
	}
	return (-1);
}

int	texture_cache_insert(t_texture_cache *cache, const char *path,
						int image_index)
{
	uint32_t		hash;
	uint32_t		index;
	t_cache_entry	*new_entry;

	if (!cache || !path)
		return (-1);
	/* Check if already exists (prevent duplicates in hash map) */
	if (texture_cache_lookup(cache, path) != -1)
		return (0);
	new_entry = malloc(sizeof(t_cache_entry));
	if (!new_entry)
		return (-1);
	new_entry->path = malloc(strlen(path) + 1);
	if (!new_entry->path)
	{
		free(new_entry);
		return (-1);
	}
	strcpy(new_entry->path, path);
	new_entry->image_index = image_index;
	hash = hash_djb2(path);
	index = hash % cache->capacity;
	new_entry->next = cache->buckets[index];
	cache->buckets[index] = new_entry;
	cache->size++;
	return (0);
}

void	texture_cache_destroy(t_texture_cache *cache)
{
	t_cache_entry	*entry;
	t_cache_entry	*temp;
	uint32_t		i;

	if (!cache)
		return ;
	i = 0;
	while (i < cache->capacity)
	{
		entry = cache->buckets[i];
		while (entry)
		{
			temp = entry->next;
			free(entry->path);
			free(entry);
			entry = temp;
		}
		i++;
	}
	free(cache->buckets);
	free(cache);
}
