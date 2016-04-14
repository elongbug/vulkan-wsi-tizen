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

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice	 pdev,
												  uint32_t			 queue_family_index,
												  struct wl_display	*display)
{
	/* TODO: */
	return VK_TRUE;
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
	VkIcdSurfaceWayland	*sfc = (VkIcdSurfaceWayland *)surface;
	tpl_display_t		*display;

	VK_CHECK(sfc->base.platform == VK_ICD_WSI_PLATFORM_WAYLAND, return VK_ERROR_DEVICE_LOST,
			 "Not supported platform surface.\n");

	display = tpl_display_get(sfc->display);
	VK_CHECK(display, return VK_ERROR_DEVICE_LOST, "tpl_display_get() failed.\n");

#if 0
	res = tpl_surface_query_supported_buffer_count(sfc->tpl.surface, &min, &max);
	VK_CHECK(res == TPL_ERROR_NONE, return VK_ERROR_DEVICE_LOST,
			 "tpl_surface_query_supported_buffer_count() failed.\n");
#endif

	/* TODO: Hard-coded. */
	caps->minImageCount = 3;
	caps->maxImageCount = 3;

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

	if (display)
		tpl_object_unreference((tpl_object_t *)display);

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
