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

VKAPI_ATTR void VKAPI_CALL
vk_DestroySurfaceKHR(VkInstance						 instance,
					 VkSurfaceKHR					 surface,
					 const VkAllocationCallbacks	*allocator)
{
	vk_surface_t *sfc = (vk_surface_t *)surface;

	if (sfc->tpl.surface)
		tpl_object_unreference((tpl_object_t *)sfc->tpl.surface);
	if (sfc->tpl.display)
		tpl_object_unreference((tpl_object_t *)sfc->tpl.display);

	vk_free(&sfc->allocator, sfc);
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateXlibSurfaceKHR(VkInstance							 instance,
						const VkXlibSurfaceCreateInfoKHR	*info,
						const VkAllocationCallbacks			*allocator,
						VkSurfaceKHR						*surface)
{
	vk_surface_t	*sfc;

	allocator = vk_get_allocator(instance, allocator);

	sfc = vk_alloc(allocator, sizeof(vk_surface_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(sfc, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	sfc->allocator = *allocator;
	sfc->platform.base.platform = VK_ICD_WSI_PLATFORM_XLIB;
	sfc->platform.xlib.dpy = info->dpy;
	sfc->platform.xlib.window = info->window;

	*surface = (VkSurfaceKHR)sfc;

	/* TODO: */

	return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice	 pdev,
											   uint32_t			 queue_family_index,
											   Display			*dpy,
											   VisualID			 visual_id)
{
	/* TODO: */
	return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateXcbSurfaceKHR(VkInstance							 instance,
					   const VkXcbSurfaceCreateInfoKHR		*info,
					   const VkAllocationCallbacks			*allocator,
					   VkSurfaceKHR							*surface)
{
	vk_surface_t	*sfc;

	allocator = vk_get_allocator(instance, allocator);

	sfc = vk_alloc(allocator, sizeof(vk_surface_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(sfc, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	sfc->allocator = *allocator;
	sfc->platform.base.platform = VK_ICD_WSI_PLATFORM_XCB;
	sfc->platform.xcb.connection = info->connection;
	sfc->platform.xcb.window = info->window;

	*surface = (VkSurfaceKHR)sfc;

	/* TODO: */

	return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice	 pdev,
											  uint32_t			 queue_family_index,
											  xcb_connection_t	*connection,
											  xcb_visualid_t	 visual_id)
{
	/* TODO: */
	return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateWaylandSurfaceKHR(VkInstance							 instance,
						   const VkWaylandSurfaceCreateInfoKHR	*info,
						   const VkAllocationCallbacks			*allocator,
						   VkSurfaceKHR							*surface)
{
	vk_surface_t	*sfc;

	allocator = vk_get_allocator(instance, allocator);

	sfc = vk_alloc(allocator, sizeof(vk_surface_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(sfc, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	memset(sfc, 0x00, sizeof(vk_surface_t));

	sfc->allocator = *allocator;
	sfc->platform.base.platform = VK_ICD_WSI_PLATFORM_WAYLAND;
	sfc->platform.wayland.display = info->display;
	sfc->platform.wayland.surface = info->surface;

	sfc->tpl.display = tpl_display_create(TPL_BACKEND_WAYLAND_VULKAN_WSI, info->display);
	VK_CHECK(sfc->tpl.display, goto error, "tpl_display_create() failed.\n");

	sfc->tpl.surface = tpl_surface_create(sfc->tpl.display, info->surface, TPL_SURFACE_TYPE_WINDOW,
										  TBM_FORMAT_ARGB8888);
	VK_CHECK(sfc->tpl.surface, goto error, "tpl_surface_create() failed.\n");

	*surface = (VkSurfaceKHR)sfc;
	return VK_SUCCESS;

error:
	vk_DestroySurfaceKHR(instance, (VkSurfaceKHR)sfc, allocator);
	return VK_ERROR_INITIALIZATION_FAILED;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice	 pdev,
												  uint32_t			 queue_family_index,
												  struct wl_display	*display)
{
	/* TODO: */
	return VK_TRUE;
}
#endif

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateDisplayPlaneSurfaceKHR(VkInstance							 instance,
								const VkDisplaySurfaceCreateInfoKHR	*info,
								const VkAllocationCallbacks			*allocator,
								VkSurfaceKHR						*surface)
{
	vk_surface_t	*sfc;

	allocator = vk_get_allocator(instance, allocator);

	sfc = vk_alloc(allocator, sizeof(vk_surface_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(sfc, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	sfc->allocator = *allocator;
	sfc->platform.base.platform = VK_ICD_WSI_PLATFORM_DISPLAY;
	sfc->platform.display.displayMode = info->displayMode;
	sfc->platform.display.planeIndex = info->planeIndex;
	sfc->platform.display.planeStackIndex = info->planeStackIndex;
	sfc->platform.display.transform = info->transform;
	sfc->platform.display.globalAlpha = info->globalAlpha;
	sfc->platform.display.alphaMode = info->alphaMode;
	sfc->platform.display.imageExtent = info->imageExtent;

	*surface = (VkSurfaceKHR)sfc;

	/* TODO: */

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice	 pdev,
									  uint32_t			 queue_family_index,
									  VkSurfaceKHR		 surface,
									  VkBool32			*supported)
{
	*supported = VK_TRUE;
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice			 pdev,
										   VkSurfaceKHR				 surface,
										   VkSurfaceCapabilitiesKHR	*caps)
{
	int min, max;
	tpl_result_t res;
	vk_surface_t *sfc = (vk_surface_t *)surface;

	res = tpl_surface_query_supported_buffer_count(sfc->tpl.surface, &min, &max);
	VK_CHECK(res == TPL_ERROR_NONE, return VK_ERROR_DEVICE_LOST,
			 "tpl_surface_query_supported_buffer_count() failed.\n");

	caps->minImageCount = min;
	caps->maxImageCount = max;
	caps->currentExtent.width = -1;
	caps->currentExtent.height = -1;
	caps->minImageExtent.width = 1;
	caps->minImageExtent.height = 1;
	caps->maxImageExtent.width = INT16_MAX;
	caps->maxImageExtent.height = INT16_MAX;
	caps->maxImageArrayLayers = 1;
	caps->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	caps->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	caps->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	caps->supportedUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	return VK_SUCCESS;
}

static VkSurfaceFormatKHR surface_formats[] = {
	{ VK_FORMAT_R8G8B8_SRGB,	VK_COLORSPACE_SRGB_NONLINEAR_KHR },
	{ VK_FORMAT_R8G8B8A8_SRGB,	VK_COLORSPACE_SRGB_NONLINEAR_KHR }
};

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice		 pdev,
									  VkSurfaceKHR			 surface,
									  uint32_t				*format_count,
									  VkSurfaceFormatKHR	*formats)
{
	/* TODO: */

	if (formats) {
		*format_count = MIN(*format_count, ARRAY_LENGTH(surface_formats));
		memcpy(formats, &surface_formats[0], sizeof(VkSurfaceFormatKHR) * (*format_count));

		if (*format_count < ARRAY_LENGTH(surface_formats))
			return VK_INCOMPLETE;
	} else {
		*format_count = ARRAY_LENGTH(surface_formats);
	}

	return VK_SUCCESS;
}

static VkPresentModeKHR present_modes[] = {
	VK_PRESENT_MODE_FIFO_KHR
};

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice	 pdev,
										   VkSurfaceKHR		 surface,
										   uint32_t			*mode_count,
										   VkPresentModeKHR	*modes)
{
	/* TODO: */

	if (modes) {
		*mode_count = MIN(*mode_count, ARRAY_LENGTH(present_modes));
		memcpy(modes, &present_modes[0], sizeof(VkPresentModeKHR) * (*mode_count));

		if (*mode_count < ARRAY_LENGTH(present_modes))
			return VK_INCOMPLETE;
	} else {
		*mode_count = ARRAY_LENGTH(present_modes);
	}

	return VK_SUCCESS;
}
