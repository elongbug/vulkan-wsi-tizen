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
#include <unistd.h>
#include <stdio.h>

#define TBM_FORMAT_0	0

#define RETURN_FORMAT(comp, opaque, pre, post, inherit)					\
	do {																\
		if (comp == VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)					\
			return TBM_FORMAT_##opaque;									\
		else if (comp == VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)		\
			return TBM_FORMAT_##pre;									\
		else if (comp == VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)	\
			return TBM_FORMAT_##post;									\
		else if (comp == VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)			\
			return TBM_FORMAT_##inherit;								\
		else															\
			return 0;													\
	} while (0)

static inline tbm_format
get_tbm_format(VkFormat format, VkCompositeAlphaFlagBitsKHR comp)
{
	switch (format) {
	/* 4 4 4 4 */
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		RETURN_FORMAT(comp, RGBX4444, RGBA4444, 0, RGBA4444);
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		RETURN_FORMAT(comp, BGRX4444, BGRA4444, 0, BGRA4444);
	/* 5 6 5 */
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		RETURN_FORMAT(comp, RGB565, RGB565, RGB565, RGB565);
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		RETURN_FORMAT(comp, BGR565, BGR565, BGR565, BGR565);
	/* 5 5 5 1 */
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		RETURN_FORMAT(comp, RGBX5551, RGBA5551, 0, RGBA5551);
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		RETURN_FORMAT(comp, BGRX5551, BGRA5551, 0, BGRA5551);
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		RETURN_FORMAT(comp, XRGB1555, ARGB1555, 0, ARGB1555);
	/* 8 8 8 */
	case VK_FORMAT_R8G8B8_UNORM:
		RETURN_FORMAT(comp, BGR888, BGR888, BGR888, BGR888);
	case VK_FORMAT_B8G8R8_UNORM:
		RETURN_FORMAT(comp, RGB888, RGB888, RGB888, RGB888);
	/* 8 8 8 8 */
	case VK_FORMAT_B8G8R8A8_UNORM:
		RETURN_FORMAT(comp, XRGB8888, ARGB8888, 0, ARGB8888);
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		RETURN_FORMAT(comp, XBGR8888, ABGR8888, 0, ABGR8888);
	/* 2 10 10 10 */
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		RETURN_FORMAT(comp, XRGB2101010, ARGB2101010, 0, ARGB2101010);
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		RETURN_FORMAT(comp, XBGR2101010, ABGR2101010, 0, ABGR2101010);
	default:
		break;
	}

	return 0;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateSwapchainKHR(VkDevice							 device,
					  const VkSwapchainCreateInfoKHR	*info,
					  const VkAllocationCallbacks		*allocator,
					  VkSwapchainKHR					*swapchain)
{
	VkResult (*init)(VkDevice, const VkSwapchainCreateInfoKHR *,
					 vk_swapchain_t *, tbm_format);
	vk_swapchain_t		*chain;
	VkResult			 error;
	uint32_t			 i;
	tbm_surface_h		*buffers;
	tbm_format			 format;
	vk_icd_t			*icd = vk_get_icd();

	switch(((VkIcdSurfaceBase *)(uintptr_t)info->surface)->platform) {
		case VK_ICD_WSI_PLATFORM_WAYLAND:
			init = swapchain_tpl_init;
			break;
		case VK_ICD_WSI_PLATFORM_DISPLAY:
			init = swapchain_tdm_init;
			break;
		default:
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	allocator = vk_get_allocator(device, allocator);

	chain = vk_alloc(allocator, sizeof(vk_swapchain_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(chain, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	memset(chain, 0x00, sizeof(vk_swapchain_t));

	chain->allocator = *allocator;
	chain->surface = info->surface;

	format = get_tbm_format(info->imageFormat, info->compositeAlpha);
	VK_CHECK(format, return VK_ERROR_SURFACE_LOST_KHR, "Not supported image format.\n");

	error = init(device, info, chain, format);
	VK_CHECK(error == VK_SUCCESS, goto done, "swapchain backend init failed.\n");

	error = chain->get_buffers(device, chain, &buffers, &chain->buffer_count);
	VK_CHECK(error == VK_SUCCESS, goto done, "swapchain backend get buffers failed.\n");

	chain->buffers = vk_alloc(&chain->allocator, chain->buffer_count * sizeof(vk_buffer_t),
							  VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(chain->buffers, goto error_mem_alloc, "vk_alloc() failed.\n");

	for (i = 0; i < chain->buffer_count; i++) {
		VkImageCreateInfo image_info = {
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			NULL,
			0,
			VK_IMAGE_TYPE_2D,
			info->imageFormat,
			{ info->imageExtent.width, info->imageExtent.height, 1 },
			1, /* mip level. */
			info->imageArrayLayers,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_LINEAR,
			info->imageUsage,
			info->imageSharingMode,
			info->queueFamilyIndexCount,
			info->pQueueFamilyIndices,
			VK_IMAGE_LAYOUT_UNDEFINED,
		};

		chain->buffers[i].tbm = buffers[i];
		icd->create_presentable_image(device, chain->buffers[i].tbm, &image_info,
									  &chain->allocator, &chain->buffers[i].image);
	}
	goto done;

error_mem_alloc:
	error = VK_ERROR_OUT_OF_HOST_MEMORY;

done:
	if (error != VK_SUCCESS) {
		if (chain->deinit)
			chain->deinit(device, chain);

		if (chain)
			vk_free(allocator, chain);

		*swapchain = VK_NULL_HANDLE;
	} else {
		*swapchain = (VkSwapchainKHR)(uintptr_t)chain;
	}

	return error;
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
	vk_swapchain_t			*chain = (vk_swapchain_t *)(uintptr_t)swapchain;
	vk_icd_t				*icd = vk_get_icd();
	PFN_vkGetDeviceProcAddr	 icd_gdpa = (PFN_vkGetDeviceProcAddr)icd->get_proc_addr(NULL, "vkGetDeviceProcAddr");

	if (icd_gdpa != VK_NULL_HANDLE) {
		PFN_vkDestroyImage		 destroy_image = (PFN_vkDestroyImage)icd_gdpa(device, "vkDestroyImage");
		if (destroy_image != VK_NULL_HANDLE) {
			uint32_t		 i;

			for (i = 0; i < chain->buffer_count; i++)
				destroy_image(device, chain->buffers[i].image, &chain->allocator);
		}
	}

	chain->deinit(device, chain);
	vk_free(&chain->allocator, chain->buffers);
	vk_free(&chain->allocator, chain);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetSwapchainImagesKHR(VkDevice		 device,
						 VkSwapchainKHR	 swapchain,
						 uint32_t		*image_count,
						 VkImage		*images)
{
	vk_swapchain_t *chain = (vk_swapchain_t *)(uintptr_t)swapchain;

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
	VkResult		 res;
	vk_swapchain_t	*chain = (vk_swapchain_t *)(uintptr_t)swapchain;
	vk_icd_t		*icd = vk_get_icd();
	tbm_surface_h	 tbm_surface;
	int				 sync;
	uint32_t		 i;

	if (icd->acquire_image)
		res = chain->acquire_image(device, chain, timeout, &tbm_surface, &sync);
	else
		res = chain->acquire_image(device, chain, timeout, &tbm_surface, NULL);
	VK_CHECK(res == VK_SUCCESS, return res, "backend acquire image failed\n.");

	for (i = 0; i < chain->buffer_count; i++) {
		if (tbm_surface == chain->buffers[i].tbm) {
			*image_index = i;
			if (icd->acquire_image)
				icd->acquire_image(device, chain->buffers[i].image, sync, semaphore, fence);

			/* TODO: We can do optimization here by returning buffer index immediatly despite the
			 * buffer is not released yet. The fence or semaphore will be signaled when
			 * wl_buffer.release actually arrives. */

			return VK_SUCCESS;
		}
	}

	return VK_ERROR_SURFACE_LOST_KHR;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_QueuePresentKHR(VkQueue					 queue,
				   const VkPresentInfoKHR	*info)
{
	uint32_t	 i;
	vk_icd_t	*icd = vk_get_icd();

	for (i = 0; i < info->swapchainCount; i++) {
		VkResult		 res;
		int				 sync_fd = -1;
		vk_swapchain_t	*chain = (vk_swapchain_t *)(uintptr_t)info->pSwapchains[i];

		if (icd->queue_signal_release_image)
			icd->queue_signal_release_image(queue, info->waitSemaphoreCount, info->pWaitSemaphores,
											chain->buffers[info->pImageIndices[i]].image, &sync_fd);

		res = chain->present_image(queue, chain,
								   chain->buffers[info->pImageIndices[i]].tbm, sync_fd);

		if (info->pResults != NULL)
			info->pResults[i] = res;
	}

	return VK_SUCCESS;
}
