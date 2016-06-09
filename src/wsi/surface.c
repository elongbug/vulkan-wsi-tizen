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
	VkIcdSurfaceWayland	*sfc = (VkIcdSurfaceWayland *)(uintptr_t)surface;
	tpl_display_t		*display;
	int min, max;
	tpl_result_t res;

	VK_CHECK(sfc->base.platform == VK_ICD_WSI_PLATFORM_WAYLAND, return VK_ERROR_DEVICE_LOST,
			 "Not supported platform surface.\n");

	display = vk_get_tpl_display(sfc->display);
	VK_CHECK(display, return VK_ERROR_DEVICE_LOST, "vk_get_tpl_display() failed.\n");

	res = tpl_display_query_supported_buffer_count_from_native_window(display, sfc->surface, &min, &max);
	VK_CHECK(res == TPL_ERROR_NONE, return VK_ERROR_DEVICE_LOST,
			 "tpl_display_query_native_window_supported_buffer_count() failed.\n");

	/* TODO: Hard-coded. */
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

	if (display)
		tpl_object_unreference((tpl_object_t *)display);

	return VK_SUCCESS;
}

#define FORMAT_ENTRY(tbm, vk, cs) { TBM_FORMAT_##tbm, { VK_FORMAT_##vk, VK_COLORSPACE_##cs }}

static const struct {
	tbm_format			tbm_format;
	VkSurfaceFormatKHR	surface_format;
} supported_formats[] = {
	/* TODO: Workaround to make tri sample run correctly. */
	FORMAT_ENTRY(RGBA8888,		B8G8R8A8_UNORM,				SRGB_NONLINEAR_KHR),

	/* TODO: Correct map between tbm formats and vulkan formats. */
	FORMAT_ENTRY(XRGB8888,		B8G8R8A8_UNORM,				SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(ARGB8888,		B8G8R8A8_UNORM,				SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(XBGR8888,		A8B8G8R8_UNORM_PACK32,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(ABGR8888,		A8B8G8R8_UNORM_PACK32,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(RGB888,		B8G8R8_UNORM,				SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(BGR888,		R8G8B8_UNORM,				SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(RGB565,		R5G6B5_UNORM_PACK16, 		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(BGR565,		B5G6R5_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(RGBX4444,		R4G4B4A4_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(RGBA4444,		R4G4B4A4_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(BGRX4444,		B4G4R4A4_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(BGRA4444,		B4G4R4A4_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(ARGB1555,		A1R5G5B5_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(XRGB1555,		A1R5G5B5_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(RGBX5551,		R5G5B5A1_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(RGBA5551,		R5G5B5A1_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(BGRX5551,		B5G5R5A1_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(BGRA5551,		B5G5R5A1_UNORM_PACK16,		SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(XRGB2101010,	A2R10G10B10_UNORM_PACK32,	SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(XBGR2101010,	A2B10G10R10_UNORM_PACK32,	SRGB_NONLINEAR_KHR),
	FORMAT_ENTRY(ABGR2101010,	A2B10G10R10_UNORM_PACK32,	SRGB_NONLINEAR_KHR),
};

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice		 pdev,
									  VkSurfaceKHR			 surface,
									  uint32_t				*format_count,
									  VkSurfaceFormatKHR	*formats)
{
	uint32_t			 tbm_format_count;
	tbm_format			*tbm_formats;
	uint32_t			 surface_format_count = 0;
	VkSurfaceFormatKHR	 surface_formats[ARRAY_LENGTH(supported_formats)];
	uint32_t			 i, j;

	if (tbm_surface_query_formats(&tbm_formats, &tbm_format_count) != TBM_SURFACE_ERROR_NONE)
		return VK_ERROR_DEVICE_LOST;

	for (i = 0; i < ARRAY_LENGTH(supported_formats); i++) {
		for (j = 0; j < tbm_format_count; j++) {
			if (tbm_formats[j] == supported_formats[i].tbm_format) {
				/* TODO Check if ICD support the format. */
				surface_formats[surface_format_count++] = supported_formats[i].surface_format;
				break;
			}
		}
	}

	free(tbm_formats);

	if (formats) {
		*format_count = MIN(*format_count, surface_format_count);
		memcpy(formats, &surface_formats[0], sizeof(VkSurfaceFormatKHR) * (*format_count));

		if (*format_count < surface_format_count)
			return VK_INCOMPLETE;
	} else {
		*format_count = surface_format_count;
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
