/*
 * Copyright © 2016 S-Core Corporation
 * Copyright © 2016-2017 Samsung Electronics co., Ltd. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "utils.h"
#include <stdlib.h>
#include <string.h>

struct vk_map_entry
{
	const void		*key;
	void			*data;
	vk_free_func_t	 free_func;
	vk_map_entry_t	*next;
};

static inline int
get_bucket_index(vk_map_t *map, const void *key)
{
	int	len = 0;
	int	hash;

	if (map->key_length_func)
		len = map->key_length_func(key);

	hash = map->hash_func(key, len);
	return hash & map->bucket_mask;
}

static inline vk_map_entry_t **
get_bucket(vk_map_t *map, const void *key)
{
	return &map->buckets[get_bucket_index(map, key)];
}

void
vk_map_init(vk_map_t				*map,
			int						 bucket_bits,
			vk_hash_func_t			 hash_func,
			vk_key_length_func_t	 key_length_func,
			vk_key_compare_func_t	 key_compare_func,
			void					*buckets)
{
	map->hash_func = hash_func;
	map->key_length_func = key_length_func;
	map->key_compare_func = key_compare_func;

	map->bucket_bits = bucket_bits;
	map->bucket_size = 1 << bucket_bits;
	map->bucket_mask = map->bucket_size - 1;

	map->buckets = buckets;
}

/* Hash functions from Thomas Wang https://gist.github.com/badboy/6267743 */
static inline int
vk_hash32(uint32_t key)
{
    key  = ~key + (key << 15);
    key ^= key >> 12;
    key += key << 2;
    key ^= key >> 4;
    key *= 2057;
    key ^= key >> 16;

    return key;
}

static inline int
vk_hash64(uint64_t key)
{
    key  = ~key + (key << 18);
    key ^= key >> 31;
    key *= 21;
    key ^= key >> 11;
    key += key << 6;
    key ^= key >> 22;

    return (int)key;
}

static int
int32_hash(const void *key, int len)
{
	return vk_hash32(*(const uint32_t *)key);
}

static int
int32_key_length(const void *key)
{
	return 4;
}

static int
int32_key_compare(const void *key0, int len0, const void *key1, int len1)
{
	uint64_t k0 = *(const uint32_t *)key0;
	uint64_t k1 = *(const uint32_t *)key1;
	return (int)(k0 - k1);
}


void
vk_map_int32_init(vk_map_t *map, int bucket_bits, void *buckets)
{
	vk_map_init(map, bucket_bits, int32_hash, int32_key_length, int32_key_compare, buckets);
}

static int
int64_hash(const void *key, int len)
{
	return vk_hash64(*(const uint64_t *)key);
}

static int
int64_key_length(const void *key)
{
	return 8;
}

static int
int64_key_compare(const void *key0, int len0, const void *key1, int len1)
{
	uint64_t k0 = *(const uint64_t *)key0;
	uint64_t k1 = *(const uint64_t *)key1;
	return (int)(k0 - k1);
}

void
vk_map_int64_init(vk_map_t *map, int bucket_bits, void *buckets)
{
	vk_map_init(map, bucket_bits, int64_hash, int64_key_length, int64_key_compare, buckets);
}

/* String hash function taken from Eina library.
   Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html)
   used by WebCore (http://webkit.org/blog/8/hashtables-part-2/) */
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
	|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
# define get16bits(d) (*((const uint16_t *)(d)))
#endif

#if !defined (get16bits)
# define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) \
					   + (uint32_t)(((const uint8_t *)(d))[0]))
#endif

static int
string_hash(const void *key, int len)
{
	int hash = len, tmp;
	int rem;
	const char *str = key;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (; len > 0; len--) {
		hash += get16bits(str);
		tmp = (get16bits(str + 2) << 11) ^ hash;
		hash = (hash << 16) ^ tmp;
		str += 2 * sizeof (uint16_t);
		hash += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
	case 3:
		hash += get16bits(str);
		hash ^= hash << 16;
		hash ^= str[sizeof (uint16_t)] << 18;
		hash += hash >> 11;
		break;

	case 2:
		hash += get16bits(str);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;

	case 1:
		hash += *str;
		hash ^= hash << 10;
		hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

static int
string_key_length(const void *key)
{
	if (!key)
		return 0;

	return (int)strlen(key) + 1;
}

static int
string_key_compare(const void *key0, int len0, const void *key1, int len1)
{
	int delta = len0 - len1;

	if (delta)
		return delta;

	return strcmp(key0, key1);
}

void
vk_map_string_init(vk_map_t *map, int bucket_bits, void *buckets)
{
	vk_map_init(map, bucket_bits, string_hash, string_key_length, string_key_compare, buckets);
}

void
vk_map_fini(vk_map_t *map)
{
	vk_map_clear(map);
}

vk_map_t *
vk_map_create(int					bucket_bits,
			  vk_hash_func_t		hash_func,
			  vk_key_length_func_t	key_length_func,
			  vk_key_compare_func_t	key_compare_func)
{
	vk_map_t	*map;
	int			 bucket_size = 1 << bucket_bits;

	map = calloc(1, sizeof(vk_map_t) + bucket_size * sizeof(vk_map_entry_t *));
	VK_CHECK(map, return NULL, "calloc() failed.\n");

	vk_map_init(map, bucket_bits, hash_func, key_length_func, key_compare_func, map + 1);
	return map;
}

vk_map_t *
vk_map_int32_create(int bucket_bits)
{
	return vk_map_create(bucket_bits, int32_hash, int32_key_length, int32_key_compare);
}

vk_map_t *
vk_map_int64_create(int bucket_bits)
{
	return vk_map_create(bucket_bits, int64_hash, int64_key_length, int64_key_compare);
}

vk_map_t *
vk_map_string_create(int bucket_bits)
{
	return vk_map_create(bucket_bits, string_hash, string_key_length, string_key_compare);
}

void
vk_map_destroy(vk_map_t *map)
{
	vk_map_fini(map);
	free(map);
}

void
vk_map_clear(vk_map_t *map)
{
	int i;

	if (!map->buckets)
		return;

	for (i = 0; i < map->bucket_size; i++)
	{
		vk_map_entry_t *curr = map->buckets[i];

		while (curr)
		{
			vk_map_entry_t *next = curr->next;

			if (curr->free_func)
				curr->free_func(curr->data);

			free(curr);
			curr = next;
		}
	}

	memset(map->buckets, 0x00, map->bucket_size * sizeof(vk_map_entry_t *));
}

void *
vk_map_get(vk_map_t *map, const void *key)
{
	vk_map_entry_t *curr = *get_bucket(map, key);

	while (curr)
	{
		int len0 = 0;
		int len1 = 0;

		if (map->key_length_func)
		{
			len0 = map->key_length_func(curr->key);
			len1 = map->key_length_func(key);
		}

		if (map->key_compare_func(curr->key, len0, key, len1) == 0)
			return curr->data;

		curr = curr->next;
	}

	return NULL;
}

void
vk_map_set(vk_map_t *map, const void *key, void *data, vk_free_func_t free_func)
{
	vk_map_entry_t	**bucket = get_bucket(map, key);
	vk_map_entry_t	 *curr = *bucket;
	vk_map_entry_t	 *prev = NULL;

	/* Find existing entry for the key. */
	while (curr)
	{
		int len0 = 0;
		int len1 = 0;

		if (map->key_length_func)
		{
			len0 = map->key_length_func(curr->key);
			len1 = map->key_length_func(key);
		}

		if (map->key_compare_func(curr->key, len0, key, len1) == 0)
		{
			/* Free previous data. */
			if (curr->free_func)
				curr->free_func(curr->data);

			if (data)
			{
				/* Set new data. */
				curr->data = data;
				curr->free_func = free_func;
			}
			else
			{
				/* Delete entry. */
				if (prev)
					prev->next = curr->next;
				else
					*bucket = curr->next;

				free(curr);
			}

			return;
		}

		prev = curr;
		curr = curr->next;
	}

	if (data == NULL)
	{
		/* Nothing to delete. */
		return;
	}

	/* Allocate a new entry. */
	curr = malloc(sizeof(vk_map_entry_t));
	VK_CHECK(curr, return, "malloc() failed.\n");

	curr->key = key;
	curr->data = data;
	curr->free_func = free_func;

	/* Insert at the head of the bucket. */
	curr->next = *bucket;
	*bucket = curr;
}
