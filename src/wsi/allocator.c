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

#include "wsi.h"
#include <stdlib.h>

static void *
default_alloc(void *data, size_t size, size_t align, VkSystemAllocationScope allocationScope)
{
	return malloc(size);
}

static void *
default_realloc(void *data, void *mem, size_t size, size_t align, VkSystemAllocationScope scope)
{
	return realloc(mem, size);
}

static void
default_free(void *data, void *mem)
{
	free(mem);
}

static const VkAllocationCallbacks default_allocator = {
	.pUserData = NULL,
	.pfnAllocation = default_alloc,
	.pfnReallocation = default_realloc,
	.pfnFree = default_free,
};

const VkAllocationCallbacks *
vk_get_allocator(void							*parent,
				 const VkAllocationCallbacks	*allocator)
{
	return &default_allocator;
}

void *
vk_alloc(const VkAllocationCallbacks	*allocator,
		 size_t							 size,
		 VkSystemAllocationScope		 scope)
{
	return allocator->pfnAllocation(allocator->pUserData, size, 1, scope);
}

void *
vk_realloc(const VkAllocationCallbacks	*allocator,
		   void							*mem,
		   size_t						 size,
		   VkSystemAllocationScope		 scope)
{
	return allocator->pfnReallocation(allocator->pUserData, mem, size, 1, scope);
}

void
vk_free(const VkAllocationCallbacks		*allocator,
		void							*mem)
{
	allocator->pfnFree(allocator->pUserData, mem);
}
