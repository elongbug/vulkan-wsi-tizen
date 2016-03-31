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

#ifndef UTILS_H
#define UTILS_H

#include <config.h>
#include <assert.h>
#include <vulkan/vulkan.h>

#if defined(__GNUC__) && __GNUC__ >= 4
#define VK_EXPORT __attribute__((visibility("default")))
#else
#define VK_EXPORT
#endif

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a)	(sizeof(a) / sizeof((a)[0]))
#endif

#define VK_ERROR(fmt, ...)											\
	do {															\
		vk_log("ERROR", __FILE__, __LINE__, fmt, ##__VA_ARGS__);	\
	} while (0)

#define VK_DEBUG(fmt, ...)											\
	do {															\
		vk_log("DEBUG", __FILE__, __LINE__, fmt, ##__VA_ARGS__);	\
	} while (0)

#define VK_CHECK(exp, action, fmt, ...)								\
	do {															\
		if (!(exp))													\
		{															\
			VK_ERROR(fmt, ##__VA_ARGS__);							\
			action;													\
		}															\
	} while (0)

#define VK_ASSERT(exp)	assert(exp)

typedef VkBool32	vk_bool_t;

void
vk_log(const char *domain, const char *file, int line, const char *format, ...);

/* Linked list. */
typedef struct vk_list vk_list_t;

#define vk_list_for_each_list(pos, head)							\
	for (pos = (head)->next;										\
		 pos != (head);												\
		 pos = pos->next)

#define vk_list_for_each_list_safe(pos, tmp, head)					\
	for (pos = (head)->next, tmp = pos->next;						\
		 pos != (head);												\
		 pos = tmp, tmp = pos->next)

#define vk_list_for_each_list_reverse(pos, head)					\
	for (pos = (head)->prev;										\
		 pos != (head);												\
		 pos = pos->prev)

#define vk_list_for_each_list_reverse_safe(pos, tmp, head)			\
	for (pos = (head)->prev, tmp = pos->prev;						\
		 pos != (head);												\
		 pos = tmp, tmp = pos->prev)

#define vk_list_for_each(pos, head, member)							\
	for (pos = vk_container_of((head)->next, pos, member);			\
		 &pos->member != (head);									\
		 pos = vk_container_of(pos->member.next, pos, member))

#define vk_list_for_each_safe(pos, tmp, head, member)				\
	for (pos = vk_container_of((head)->next, pos, member),			\
		 tmp = vk_container_of((pos)->member.next, tmp, member);	\
		 &pos->member != (head);									\
		 pos = tmp,													\
		 tmp = vk_container_of(pos->member.next, tmp, member))

#define vk_list_for_each_reverse(pos, head, member)					\
	for (pos = vk_container_of((head)->prev, pos, member);			\
		 &pos->member != (head);									\
		 pos = vk_container_of(pos->member.prev, pos, member))

#define vk_list_for_each_reverse_safe(pos, tmp, head, member)		\
	for (pos = vk_container_of((head)->prev, pos, member),			\
		 tmp = vk_container_of((pos)->member.prev, tmp, member);	\
		 &pos->member != (head);									\
		 pos = tmp,													\
		 tmp = vk_container_of(pos->member.prev, tmp, member))

struct vk_list
{
	vk_list_t *prev;
	vk_list_t *next;
	void      *item;
};

static inline void
vk_list_init(vk_list_t *list)
{
	list->prev = list;
	list->next = list;
	list->item = NULL;
}

static inline void
vk_list_insert(vk_list_t *list, vk_list_t *elm)
{
	elm->prev = list;
	elm->next = list->next;
	list->next = elm;
	elm->next->prev = elm;
}

static inline void
vk_list_remove(vk_list_t *list)
{
	list->prev->next = list->next;
	list->next->prev = list->prev;
	list->prev = NULL;
	list->next = NULL;
}

static inline vk_bool_t
vk_list_empty(const vk_list_t *list)
{
	return list->next == list;
}

static inline void
vk_list_insert_list(vk_list_t *list, vk_list_t *other)
{
	if (vk_list_empty(other))
		return;

	other->next->prev = list;
	other->prev->next = list->next;
	list->next->prev = other->prev;
	list->next = other->next;
}

#endif	/* UTILS_H */
