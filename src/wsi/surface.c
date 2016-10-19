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

static VkResult
tpl_get_surface_capabilities(VkIcdSurfaceWayland		*sfc,
							 VkSurfaceCapabilitiesKHR	*caps)
{
	tpl_display_t		*display;
	int					 min, max;
	tpl_result_t		 res;

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

	caps->maxImageExtent.width = 4096; // Temporal maxImageExtent.width value for Tizen
	caps->maxImageExtent.height = 4096; // Temporal maxImageExtent.height value for Tizen

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

static VkResult
tdm_get_surface_capabilities(VkIcdSurfaceDisplay		*sfc,
							 VkSurfaceCapabilitiesKHR	*caps)
{
	int32_t			 minw, minh, maxw, maxh;
	tdm_error			 tdm_err;
	vk_display_mode_t	*disp_mode = (vk_display_mode_t *)(uintptr_t)sfc->displayMode;
	vk_display_t		*disp = disp_mode->display;

	tdm_err = tdm_output_get_available_size(disp->tdm_output, &minw, &minh,
											&maxw, &maxh, NULL);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_DEVICE_LOST,
			 "tdm_output_get_available_size failed.\n");

	/* TODO: Hard-coded. */
	caps->minImageCount = 2;
	caps->maxImageCount = 3;

	caps->currentExtent.width = sfc->imageExtent.width;
	caps->currentExtent.height = sfc->imageExtent.height;

	caps->minImageExtent.width = minw;
	caps->minImageExtent.height = minh;

	caps->maxImageExtent.width = maxw;
	caps->maxImageExtent.height = maxh;

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

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice			 pdev,
										   VkSurfaceKHR				 surface,
										   VkSurfaceCapabilitiesKHR	*caps)
{
	switch (((VkIcdSurfaceBase *)(uintptr_t)surface)->platform) {
		case VK_ICD_WSI_PLATFORM_WAYLAND:
			return tpl_get_surface_capabilities((VkIcdSurfaceWayland *)
												(uintptr_t)surface, caps);
		case VK_ICD_WSI_PLATFORM_DISPLAY:
			return tdm_get_surface_capabilities((VkIcdSurfaceDisplay *)
												(uintptr_t)surface, caps);
		default:
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
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

static VkResult
tpl_get_surface_formats(VkIcdSurfaceWayland	*sfc,
						uint32_t			*format_count,
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

static VkResult
tdm_get_surface_formats(VkIcdSurfaceDisplay	*sfc,
						uint32_t			*format_count,
						VkSurfaceFormatKHR	*formats)
{
	int					 tbm_format_count, j;
	const tbm_format	*tbm_formats;
	uint32_t			 surface_format_count = 0, i;
	VkSurfaceFormatKHR	 surface_formats[ARRAY_LENGTH(supported_formats)];
	tdm_error			 tdm_err;
	vk_display_mode_t	*disp_mode = (vk_display_mode_t *)(uintptr_t)sfc->displayMode;
	tdm_layer			*layer = disp_mode->display->pdev->planes[sfc->planeIndex].tdm_layer;

	tdm_err = tdm_layer_get_available_formats(layer, &tbm_formats, &tbm_format_count);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_DEVICE_LOST,
			 "tdm_layer_get_available_formats failed.\n");

	for (i = 0; i < ARRAY_LENGTH(supported_formats); i++) {
		for (j = 0; j < tbm_format_count; j++) {
			if (tbm_formats[j] == supported_formats[i].tbm_format) {
				/* TODO Check if ICD support the format. */
				surface_formats[surface_format_count++] = supported_formats[i].surface_format;
				break;
			}
		}
	}

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

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice		 pdev,
									  VkSurfaceKHR			 surface,
									  uint32_t				*format_count,
									  VkSurfaceFormatKHR	*formats)
{
	switch (((VkIcdSurfaceBase *)(uintptr_t)surface)->platform) {
		case VK_ICD_WSI_PLATFORM_WAYLAND:
			return tpl_get_surface_formats((VkIcdSurfaceWayland *)
										   (uintptr_t)surface, format_count, formats);
		case VK_ICD_WSI_PLATFORM_DISPLAY:
			return tdm_get_surface_formats((VkIcdSurfaceDisplay *)
										   (uintptr_t)surface, format_count, formats);
		default:
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static VkResult
tpl_get_surface_present_modes(VkIcdSurfaceWayland	*sfc,
							  uint32_t				*mode_count,
							  VkPresentModeKHR		*modes)
{
	tpl_display_t		*display;
	tpl_result_t		 res;
	int					 tpl_support_modes;
	uint32_t			 support_mode_cnt = 0;

	VK_CHECK(sfc->base.platform == VK_ICD_WSI_PLATFORM_WAYLAND, return VK_ERROR_DEVICE_LOST,
			 "Not supported platform surface.\n");

	display = vk_get_tpl_display(sfc->display);
	VK_CHECK(display, return VK_ERROR_DEVICE_LOST, "vk_get_tpl_display() failed.\n");

	res = tpl_display_query_supported_present_modes_from_native_window(display, sfc->surface,
																	   &tpl_support_modes);
	tpl_object_unreference((tpl_object_t *)display);
	VK_CHECK(res == TPL_ERROR_NONE, return VK_ERROR_DEVICE_LOST,
			 "tpl_display_query_native_window_supported_buffer_count() failed.\n");

	if (tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_MAILBOX)
		support_mode_cnt++;
	if (tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_FIFO)
		support_mode_cnt++;
	if (tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_IMMEDIATE)
		support_mode_cnt++;
	if (tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_FIFO_RELAXED)
		support_mode_cnt++;

	if (modes) {
		uint32_t i = 0;
		*mode_count = MIN(*mode_count, support_mode_cnt);

		if (i < *mode_count && tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_FIFO)
			modes[i++] = VK_PRESENT_MODE_FIFO_KHR;
		if (i < *mode_count && tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_MAILBOX)
			modes[i++] = VK_PRESENT_MODE_MAILBOX_KHR;
		if (i < *mode_count && tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_IMMEDIATE)
			modes[i++] = VK_PRESENT_MODE_IMMEDIATE_KHR;
		if (i < *mode_count && tpl_support_modes & TPL_DISPLAY_PRESENT_MODE_FIFO_RELAXED)
			modes[i++] = VK_PRESENT_MODE_FIFO_RELAXED_KHR;

		if (*mode_count < support_mode_cnt)
			return VK_INCOMPLETE;
	} else {
		*mode_count = support_mode_cnt;
	}

	return VK_SUCCESS;
}

static VkResult
tdm_get_surface_present_modes(VkIcdSurfaceDisplay	*sfc,
							  uint32_t				*mode_count,
							  VkPresentModeKHR		*modes)
{
	/* TODO: hard-coded
	 *	need implements swapchain
	 */
	if (modes) {
		uint32_t i = 0;

		if (i < *mode_count)
			modes[i++] = VK_PRESENT_MODE_FIFO_KHR;
		if (i < *mode_count)
			modes[i++] = VK_PRESENT_MODE_MAILBOX_KHR;
		if (i < *mode_count)
			modes[i++] = VK_PRESENT_MODE_IMMEDIATE_KHR;
		if (i < *mode_count)
			modes[i++] = VK_PRESENT_MODE_FIFO_RELAXED_KHR;

		if (*mode_count < 4)
			return VK_INCOMPLETE;
	} else {
		*mode_count = 4;
	}

	return VK_SUCCESS;
}


VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice	 pdev,
										   VkSurfaceKHR		 surface,
										   uint32_t			*mode_count,
										   VkPresentModeKHR	*modes)
{
	switch (((VkIcdSurfaceBase *)(uintptr_t)surface)->platform) {
		case VK_ICD_WSI_PLATFORM_WAYLAND:
			return tpl_get_surface_present_modes((VkIcdSurfaceWayland *)
												 (uintptr_t)surface, mode_count, modes);
		case VK_ICD_WSI_PLATFORM_DISPLAY:
			return tdm_get_surface_present_modes((VkIcdSurfaceDisplay *)
												 (uintptr_t)surface, mode_count, modes);
		default:
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
