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
	vk_swapchain_t *chain;
	vk_surface_t *sfc;
	tbm_format format;
	tpl_result_t res;

	switch (info->imageFormat) {
		case VK_FORMAT_R8G8B8_SRGB:
			format = TBM_FORMAT_XRGB8888;
			break;
		case VK_FORMAT_R8G8B8A8_SRGB:
			format = TBM_FORMAT_ARGB8888;
			break;
		default:
			format = TBM_FORMAT_ARGB8888;
	}

	allocator = vk_get_allocator(device, allocator);

	chain = vk_alloc(allocator, sizeof(vk_swapchain_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(chain, return VK_ERROR_OUT_OF_HOST_MEMORY, "swapchain vk_alloc() failed.\n");

	memset(chain, 0x00, sizeof(vk_swapchain_t));

	sfc = (vk_surface_t *)info->surface;
	res = tpl_surface_create_swapchain(sfc->tpl.surface, format, info->imageExtent.width,
									   info->imageExtent.height, info->minImageCount);
	VK_CHECK(res == TPL_ERROR_NONE, goto error, "tpl_surface_create_swapchain() failed.\n");

	chain->allocator = *allocator;
	chain->vk_surface = sfc;

	*swapchain = (VkSwapchainKHR)chain;
	return VK_SUCCESS;

error:
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

	tpl_surface_destroy_swapchain(chain->vk_surface->tpl.surface);
	free(chain->buffers);

	vk_free(&chain->allocator, chain);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetSwapchainImagesKHR(VkDevice		 device,
						 VkSwapchainKHR	 swapchain,
						 uint32_t		*image_count,
						 VkImage		*images)
{
	vk_swapchain_t *chain = (vk_swapchain_t *)swapchain;

	if (chain->buffer_count == 0) {
		tpl_result_t res =tpl_surface_get_swapchain_buffers(chain->vk_surface->tpl.surface,
															&chain->buffers, &chain->buffer_count);
		VK_CHECK(res == TPL_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
				 "tpl_surface_get_swapchain_buffers() failed\n.");
	}

	if (images) {
		int32_t i;
		for (i = 0; i < (int32_t)*image_count && i < chain->buffer_count; i++) {
			/* TODO: tbm_surface_h to VkImage */
			images[i] = (VkImage)chain->buffers[i];
		}

		*image_count = i;

		if (i < chain->buffer_count)
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

	int i;
	tbm_surface_h next;
	vk_swapchain_t *chain = (vk_swapchain_t *)swapchain;

	next = tpl_surface_dequeue_buffer(chain->vk_surface->tpl.surface);
	VK_CHECK(next, return VK_ERROR_SURFACE_LOST_KHR,
			 "tpl_surface_get_swapchain_buffers() failed\n.");

	for (i = 0; i < chain->buffer_count; i++) {
		if (next == chain->buffers[chain->buffer_index])
			break;

		if (++chain->buffer_index == chain->buffer_count)
			chain->buffer_index = 0;
	}

	VK_CHECK(i == chain->buffer_count, return VK_ERROR_SURFACE_LOST_KHR,
			 "tpl_surface_get_swapchain_buffers() return new buffer[%p]\n.", next);

	*image_index = chain->buffer_index;

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_QueuePresentKHR(VkQueue					 queue,
				   const VkPresentInfoKHR	*info)
{
	uint32_t i;
	uint32_t buf_index;
	vk_swapchain_t *chain;
	tpl_result_t res;

	for (i = 0; i < info->swapchainCount; i++) {
		chain = (vk_swapchain_t *)info->pSwapchains[i];
		buf_index = info->pImageIndices[i];

		res = tpl_surface_enqueue_buffer(chain->vk_surface->tpl.surface,
										 chain->buffers[buf_index]);

		if (info->pResults != NULL) {
			if (res != TPL_ERROR_NONE)
				info->pResults[i] = VK_ERROR_DEVICE_LOST;
			else
				info->pResults[i] = VK_SUCCESS;
		}
	}
	return VK_SUCCESS;
}
