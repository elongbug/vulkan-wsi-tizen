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
#include <string.h>

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateSwapchainKHR(VkDevice							 device,
					  const VkSwapchainCreateInfoKHR	*info,
					  const VkAllocationCallbacks		*allocator,
					  VkSwapchainKHR					*swapchain)
{
	vk_swapchain_t		*chain;
	tbm_format			 format;
	tpl_result_t		 res;
	VkIcdSurfaceWayland	*surface = (VkIcdSurfaceWayland *)info->surface;
	int					 buffer_count, i;
	tbm_surface_h		*buffers;

	VK_ASSERT(surface->base.platform == VK_ICD_WSI_PLATFORM_WAYLAND);

	switch (info->imageFormat) {
		case VK_FORMAT_R8G8B8_SRGB:
			format = TBM_FORMAT_XRGB8888;
			break;
		case VK_FORMAT_R8G8B8A8_SRGB:
			format = TBM_FORMAT_ARGB8888;
			break;
		default:
			return VK_ERROR_SURFACE_LOST_KHR;
	}

	allocator = vk_get_allocator(device, allocator);

	chain = vk_alloc(allocator, sizeof(vk_swapchain_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(chain, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	memset(chain, 0x00, sizeof(vk_swapchain_t));

	chain->allocator = *allocator;
	chain->surface = info->surface;

	/* Don't check NULL for display and window. There might be default ones for some systems. */

	chain->tpl_display = tpl_display_get(surface->display);
	VK_CHECK(chain->tpl_display, goto error, "tpl_display_get() failed.\n");

	chain->tpl_surface = tpl_surface_create(chain->tpl_display, surface->surface,
											TPL_SURFACE_TYPE_WINDOW, format);
	VK_CHECK(chain->tpl_surface, goto error, "tpl_surface_create() failed.\n");

	res = tpl_surface_create_swapchain(chain->tpl_surface, format,
									   info->imageExtent.width, info->imageExtent.height,
									   info->minImageCount);
	VK_CHECK(res == TPL_ERROR_NONE, goto error, "tpl_surface_create_swapchain() failed.\n");

	/* Initialize swapchain buffers. */
	res = tpl_surface_get_swapchain_buffers(chain->tpl_surface, &buffers, &buffer_count);
	VK_CHECK(res == TPL_ERROR_NONE, goto error, "tpl_surface_get_swapchain_buffers() failed.\n");

	chain->buffers = vk_alloc(allocator, buffer_count * sizeof(vk_buffer_t),
							  VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(chain->buffers, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	for (i = 0; i < buffer_count; i++) {
		chain->buffers[i].tbm = buffers[i];

		/* TODO: Create VkImage from tbm_surface_h. */
		chain->buffers[i].image = (VkImage)buffers[i];
	}

	*swapchain = (VkSwapchainKHR)chain;
	return VK_SUCCESS;

error:
	if (chain->tpl_display)
		tpl_object_unreference((tpl_object_t *)chain->tpl_display);

	if (chain->tpl_surface)
		tpl_object_unreference((tpl_object_t *)chain->tpl_surface);

	if (chain->buffers)
		vk_free(allocator, chain->buffers);

	vk_free(allocator, chain);
	return VK_ERROR_DEVICE_LOST;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateSharedSwapchainsKHR(VkDevice						 device,
							 uint32_t						 swapchain_count,
							 const VkSwapchainCreateInfoKHR	*infos,
							 const VkAllocationCallbacks	*allocator,
							 VkSwapchainKHR					*swapchains)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_DestroySwapchainKHR(VkDevice						 device,
					   VkSwapchainKHR				 swapchain,
					   const VkAllocationCallbacks	*allocator)
{
	vk_swapchain_t *chain = (vk_swapchain_t *)swapchain;

	tpl_surface_destroy_swapchain(chain->tpl_surface);
	free(chain->buffers);

	if (chain->tpl_display)
		tpl_object_unreference((tpl_object_t *)chain->tpl_display);

	if (chain->tpl_surface)
		tpl_object_unreference((tpl_object_t *)chain->tpl_surface);

	vk_free(&chain->allocator, chain);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetSwapchainImagesKHR(VkDevice		 device,
						 VkSwapchainKHR	 swapchain,
						 uint32_t		*image_count,
						 VkImage		*images)
{
	vk_swapchain_t *chain = (vk_swapchain_t *)swapchain;

	if (images) {
		uint32_t i;

		*image_count = MIN(*image_count, chain->buffer_count);

		for (i = 0; i < *image_count; i++)
			images[i] = chain->buffers[i].image;

		if (*image_count < chain->buffer_count)
			return VK_INCOMPLETE;
	} else {
		*image_count = chain->buffer_count;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_AcquireNextImageKHR(VkDevice			 device,
					   VkSwapchainKHR	 swapchain,
					   uint64_t			 timeout,
					   VkSemaphore		 semaphore,
					   VkFence			 fence,
					   uint32_t			*image_index)
{
	/* TODO: apply timeout, semaphore, fence */

	uint32_t		 i;
	tbm_surface_h	 next;
	vk_swapchain_t	*chain = (vk_swapchain_t *)swapchain;

	next = tpl_surface_dequeue_buffer(chain->tpl_surface);
	VK_CHECK(next, return VK_ERROR_SURFACE_LOST_KHR, "tpl_surface_dequeue_buffers() failed\n.");

	for (i = 0; i < chain->buffer_count; i++) {
		if (next == chain->buffers[i].tbm) {
			*image_index = i;
			return VK_SUCCESS;
		}
	}

	return VK_ERROR_SURFACE_LOST_KHR;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_QueuePresentKHR(VkQueue					 queue,
				   const VkPresentInfoKHR	*info)
{
	uint32_t i;

	for (i = 0; i < info->swapchainCount; i++) {
		tpl_result_t	 res;
		vk_swapchain_t	*chain = (vk_swapchain_t *)info->pSwapchains[i];

		res = tpl_surface_enqueue_buffer(chain->tpl_surface,
										 chain->buffers[info->pImageIndices[i]].tbm);

		if (info->pResults != NULL)
			info->pResults[i] = res == TPL_ERROR_NONE ? VK_SUCCESS : VK_ERROR_DEVICE_LOST;
	}

	return VK_SUCCESS;
}
