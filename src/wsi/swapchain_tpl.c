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

typedef struct vk_swapchain_tpl vk_swapchain_tpl_t;

struct vk_swapchain_tpl {
	tpl_display_t			*tpl_display;
	tpl_surface_t			*tpl_surface;
	tbm_surface_h			*buffers;
};

static VkResult
swapchain_tpl_queue_present_image(VkQueue					 queue,
								  vk_swapchain_t			*chain,
								  tbm_surface_h				 tbm_surface,
								  int						 sync_fd)
{
	tpl_result_t		 res;
	vk_swapchain_tpl_t	*swapchain_tpl = chain->backend_data;

	res = tpl_surface_enqueue_buffer_with_damage_and_sync(swapchain_tpl->tpl_surface,
														  tbm_surface, 0, NULL, sync_fd);
	return res == TPL_ERROR_NONE ? VK_SUCCESS : VK_ERROR_DEVICE_LOST;
}

static VkResult
swapchain_tpl_acquire_next_image(VkDevice			 device,
								 vk_swapchain_t		*chain,
								 uint64_t			 timeout,
								 tbm_surface_h		*tbm_surface,
								 int				*sync)
{
	vk_swapchain_tpl_t	*swapchain_tpl = chain->backend_data;

	if (sync) {
		*tbm_surface = tpl_surface_dequeue_buffer_with_sync(swapchain_tpl->tpl_surface,
															timeout, sync);
		if (*tbm_surface == NULL)
			return VK_TIMEOUT;
	} else {
		*tbm_surface = tpl_surface_dequeue_buffer(swapchain_tpl->tpl_surface);
		VK_CHECK(*tbm_surface, return VK_ERROR_SURFACE_LOST_KHR, "tpl_surface_dequeue_buffers() failed.\n");
	}

	return VK_SUCCESS;
}

static void
swapchain_tpl_deinit(VkDevice		 device,
				 vk_swapchain_t *chain)
{
	vk_swapchain_tpl_t	*swapchain_tpl = chain->backend_data;

	if (swapchain_tpl) {
		tpl_surface_destroy_swapchain(swapchain_tpl->tpl_surface);

		if (swapchain_tpl->tpl_surface)
			tpl_object_unreference((tpl_object_t *)swapchain_tpl->tpl_surface);

		if (swapchain_tpl->tpl_display)
			tpl_object_unreference((tpl_object_t *)swapchain_tpl->tpl_display);

		if (swapchain_tpl->buffers)
			free(swapchain_tpl->buffers);
		vk_free(&chain->allocator, swapchain_tpl);
	}
}

static VkResult
swapchain_tpl_get_buffers(VkDevice			 device,
						  vk_swapchain_t	*chain,
						  tbm_surface_h	   **buffers,
						  uint32_t			*buffer_count)
{
	tpl_result_t		 res;
	int					 buffer_cnt;
	VkResult			 error = VK_SUCCESS;
	vk_swapchain_tpl_t	*swapchain_tpl = chain->backend_data;

	/* Initialize swapchain buffers. */
	res = tpl_surface_get_swapchain_buffers(swapchain_tpl->tpl_surface,
											&swapchain_tpl->buffers, &buffer_cnt);
	VK_CHECK(res == TPL_ERROR_NONE, goto done, "tpl_surface_get_swapchain_buffers() failed.\n");

	*buffers = swapchain_tpl->buffers;
	*buffer_count = buffer_cnt;

done:
	if (res == TPL_ERROR_OUT_OF_MEMORY)
		error = VK_ERROR_OUT_OF_DEVICE_MEMORY;
	else if (res != TPL_ERROR_NONE)
		error = VK_ERROR_SURFACE_LOST_KHR;

	return error;
}

VkResult
swapchain_tpl_init(VkDevice							 device,
				   const VkSwapchainCreateInfoKHR	*info,
				   vk_swapchain_t					*chain,
				   tbm_format						 format)
{
	tpl_result_t		 res;
	VkIcdSurfaceBase	*surface = (VkIcdSurfaceBase *)(uintptr_t)info->surface;
	vk_swapchain_tpl_t	*swapchain_tpl;
	tpl_handle_t		 native_window;
	int					 tpl_present_mode;

	VkResult error = VK_ERROR_DEVICE_LOST;

	swapchain_tpl = vk_alloc(&chain->allocator, sizeof(vk_swapchain_tpl_t),
							 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(swapchain_tpl, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");
	memset(swapchain_tpl, 0x00, sizeof(*swapchain_tpl));
	chain->backend_data = swapchain_tpl;

	/* Don't check NULL for display and window. There might be default ones for some systems. */

	swapchain_tpl->tpl_display = vk_get_tpl_display(surface);
	VK_CHECK(swapchain_tpl->tpl_display, goto error, "vk_get_tpl_display() failed.\n");
	native_window = vk_get_tpl_native_window(surface);

	swapchain_tpl->tpl_surface = tpl_surface_create(swapchain_tpl->tpl_display,
														   native_window,
														   TPL_SURFACE_TYPE_WINDOW, format);
	VK_CHECK(swapchain_tpl->tpl_surface, goto error, "tpl_surface_create() failed.\n");

	switch(info->presentMode) {
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			tpl_present_mode = TPL_DISPLAY_PRESENT_MODE_IMMEDIATE;
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			tpl_present_mode = TPL_DISPLAY_PRESENT_MODE_MAILBOX;
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			tpl_present_mode = TPL_DISPLAY_PRESENT_MODE_FIFO;
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			tpl_present_mode = TPL_DISPLAY_PRESENT_MODE_FIFO_RELAXED;
			break;
		default:
			VK_DEBUG("Unsupported present mode: 0x%x\n", info->presentMode);
			goto error;
	}

	res = tpl_surface_create_swapchain(swapchain_tpl->tpl_surface, format,
									   info->imageExtent.width, info->imageExtent.height,
									   info->minImageCount, tpl_present_mode);
	if (res == TPL_ERROR_OUT_OF_MEMORY) {
		error = VK_ERROR_OUT_OF_DEVICE_MEMORY;
		VK_ERROR("tpl_surface_create_swapchain() failed.\n");
		goto error;
	}
	VK_CHECK(res == TPL_ERROR_NONE, goto error, "tpl_surface_create_swapchain() failed.\n");

	chain->get_buffers = swapchain_tpl_get_buffers;
	chain->deinit = swapchain_tpl_deinit;
	chain->acquire_image = swapchain_tpl_acquire_next_image;
	chain->present_image = swapchain_tpl_queue_present_image;

	return VK_SUCCESS;

error:
	if (swapchain_tpl->tpl_display)
		tpl_object_unreference((tpl_object_t *)swapchain_tpl->tpl_display);

	if (swapchain_tpl->tpl_surface)
		tpl_object_unreference((tpl_object_t *)swapchain_tpl->tpl_surface);

	return error;
}
