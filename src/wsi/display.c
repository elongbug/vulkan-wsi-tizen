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

static void
add_tdm_layer(vk_physical_device_t *pdev, tdm_layer *layer,
			  vk_display_t *display, tdm_output *output)
{
	int zpos;

	vk_display_plane_t *plane = &pdev->planes[pdev->plane_count];

	plane->pdev = pdev;
	plane->tdm_layer = layer;

	plane->supported_display_count = 1;
	plane->supported_displays[0] = display;

	/* TODO: Map layer zpos into positive integer range between [0, NUM_LAYERS].*/
	plane->current_display = display;

	tdm_layer_get_zpos(layer, &zpos);
	plane->current_stack_index = zpos;

	plane->prop.currentDisplay = VK_TO_HANDLE(VkDisplayKHR, plane->current_display);
	plane->prop.currentStackIndex = plane->current_stack_index;

	pdev->plane_count++;
}

static void
plane_fini(vk_display_plane_t *plane)
{
	/* Do Nothing */
}

static void
add_tdm_output(vk_physical_device_t *pdev, tdm_output *output)
{
	vk_display_t 			*display = &pdev->displays[pdev->display_count];
	const char 				*str;
	unsigned int 			 w, h;
	int						 r_w, r_h;
	int						 count, i;
	const tdm_output_mode  *modes;
	tdm_error				 error;

	display->pdev = pdev;
	display->tdm_output = output;

	display->built_in_modes = NULL;
	display->built_in_mode_count = 0;
	/* Initialize modes. */
	tdm_output_get_available_modes(output, &modes, &count);
	if (count > 0) {
		display->built_in_modes = calloc(count, sizeof(vk_display_mode_t));
		VK_CHECK(display->built_in_modes, return, "calloc() failed.\n");

		for (i = 0; i < count; i++) {
			display->built_in_modes[i].display = display;
			display->built_in_modes[i].prop.displayMode =
				VK_TO_HANDLE(VkDisplayModeKHR, &display->built_in_modes[i]);
			display->built_in_modes[i].prop.parameters.visibleRegion.width = modes[i].hdisplay;
			display->built_in_modes[i].prop.parameters.visibleRegion.height = modes[i].vdisplay;
			display->built_in_modes[i].prop.parameters.refreshRate = modes[i].vrefresh;
		}
		display->built_in_mode_count = count;
	}

	display->custom_mode_count = 0;
	display->custom_modes = NULL;

	/* Initialize prop. */
	display->prop.display = VK_TO_HANDLE(VkDisplayKHR, display);

	error = tdm_output_get_model_info(output, NULL, NULL, &str);
	VK_CHECK(error == TDM_ERROR_NONE, return, "tdm_output_get_model_info failed.\n");
	display->prop.displayName = strdup(str);

	error = tdm_output_get_physical_size(output, &w, &h);
	VK_CHECK(error == TDM_ERROR_NONE, return, "tdm_output_get_tdm_output_get_physical_size failed.\n");
	display->prop.physicalDimensions.width = w;
	display->prop.physicalDimensions.height = h;

	error = tdm_output_get_available_size(output, NULL, NULL, &r_w, &r_h, NULL);
	VK_CHECK(error == TDM_ERROR_NONE, return, "tdm_output_get_available_size failed.\n");
	display->prop.physicalResolution.width = r_w;
	display->prop.physicalResolution.height = r_h;

	/* TODO: Transform */
	display->prop.supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	/* TODO: Changing Z pos is only allowed for video layers. */
	display->prop.planeReorderPossible = VK_FALSE;

	display->prop.persistentContent = VK_FALSE;

	/* Add layers */
	error = tdm_output_get_layer_count(output, &count);

	for (i = 0; i < count; i++) {
		tdm_layer *layer = tdm_output_get_layer(output, i, &error);
		add_tdm_layer(pdev, layer, display, output);
	}

	/* Finally increase display count. */
	pdev->display_count++;
}

static void
display_fini(vk_display_t *display)
{
	if (display->built_in_modes)
		free(display->built_in_modes);

	if (display->custom_modes)
		free(display->custom_modes);
}

void
vk_physical_device_fini_display(vk_physical_device_t *pdev)
{
	uint32_t i;

	for (i = 0; i < pdev->display_count; i++)
		display_fini(&pdev->displays[i]);

	for (i = 0; i < pdev->plane_count; i++)
		plane_fini(&pdev->planes[i]);

	if (pdev->tdm_display)
		tdm_display_deinit(pdev->tdm_display);

	pdev->tdm_display = NULL;
	pdev->display_count = 0;
	pdev->plane_count = 0;
}

VkBool32
vk_physical_device_init_display(vk_physical_device_t *pdev)
{
	tdm_error			 err;
	int					 output_count, i;

	pdev->tdm_display = NULL;
	pdev->display_count = 0;
	pdev->plane_count = 0;

	/* Initialize TDM display. */
	pdev->tdm_display = tdm_display_init(&err);
	VK_CHECK(err == TDM_ERROR_NONE, goto error, "tdm_display_init() failed.\n");

	/* Get total output count. */
	err = tdm_display_get_output_count(pdev->tdm_display, &output_count);
	VK_CHECK(err == TDM_ERROR_NONE, goto error, "tdm_display_get_output_count() failed.\n");

	/* Add TDM outputs. */
	for (i = 0; i < output_count; i++) {
		tdm_output *output = tdm_display_get_output(pdev->tdm_display, i, &err);
		VK_CHECK(err == TDM_ERROR_NONE, goto error, "tdm_display_get_output() failed.\n");
		add_tdm_output(pdev, output);
	}

	return VK_TRUE;

error:
	vk_physical_device_fini_display(pdev);
	return VK_FALSE;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice		 pdev,
										 uint32_t				*prop_count,
										 VkDisplayPropertiesKHR	*props)
{
	vk_physical_device_t	*phydev = vk_get_physical_device(pdev);
	uint32_t				 i;

	if (!props) {
		*prop_count = phydev->display_count;
		return VK_SUCCESS;
	}

	*prop_count = MIN(*prop_count, phydev->display_count);

	for (i = 0; i < *prop_count; i++)
		props[i] = phydev->displays[i].prop;

	if (*prop_count < phydev->display_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice				 pdev,
											  uint32_t						*prop_count,
											  VkDisplayPlanePropertiesKHR	*props)
{
	vk_physical_device_t	*phydev = vk_get_physical_device(pdev);
	uint32_t				 i;

	if (!props) {
		*prop_count = phydev->plane_count;
		return VK_SUCCESS;
	}

	*prop_count = MIN(*prop_count, phydev->plane_count);

	for (i = 0; i < *prop_count; i++)
		props[i] = phydev->planes[i].prop;

	if (*prop_count < phydev->plane_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice	 pdev,
									   uint32_t			 plane_index,
									   uint32_t			*display_count,
									   VkDisplayKHR		*displays)
{
	vk_physical_device_t	*phydev = vk_get_physical_device(pdev);
	vk_display_plane_t		*plane = &phydev->planes[plane_index];
	uint32_t				 i;

	if (!displays) {
		*display_count = plane->supported_display_count;
		return VK_SUCCESS;
	}

	*display_count = MIN(*display_count, plane->supported_display_count);

	for (i = 0; i < *display_count; i++)
		displays[i] = VK_TO_HANDLE(VkDisplayKHR, plane->supported_displays[i]);

	if (*display_count < plane->supported_display_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayModePropertiesKHR(VkPhysicalDevice				 pdev,
							   VkDisplayKHR					 display,
							   uint32_t						*prop_count,
							   VkDisplayModePropertiesKHR	*props)
{
	vk_display_t			*dpy = VK_TO_POINTER(vk_display_t, display);
	uint32_t				 i;

	if (!props) {
		*prop_count = dpy->built_in_mode_count;
		return VK_SUCCESS;
	}

	*prop_count = MIN(*prop_count, dpy->built_in_mode_count);

	for (i = 0; i < *prop_count; i++)
		props[i] = dpy->built_in_modes[i].prop;

	if (*prop_count < dpy->built_in_mode_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateDisplayModeKHR(VkPhysicalDevice					 pdev,
						VkDisplayKHR						 display,
						const VkDisplayModeCreateInfoKHR	*info,
						const VkAllocationCallbacks			*allocator,
						VkDisplayModeKHR					*mode)
{
	/* TODO: */
	return VK_ERROR_INITIALIZATION_FAILED;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice				 pdev,
								  VkDisplayModeKHR				 mode,
								  uint32_t						 plane_index,
								  VkDisplayPlaneCapabilitiesKHR	*caps)
{
	int min_w, min_h, max_w, max_h;
	tdm_error tdm_err;
	vk_physical_device_t	*phydev = vk_get_physical_device(pdev);
	vk_display_plane_t		*plane = &phydev->planes[plane_index];
	vk_display_t			*disp = plane->current_display;

	tdm_err = tdm_output_get_available_size(disp->tdm_output, &min_w, &min_h,
											&max_w, &max_h, NULL);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_DEVICE_LOST, "tdm_output_get_available_size failed.\n");

	memset(caps, 0x00, sizeof(VkDisplayPlaneCapabilitiesKHR));

	/* TODO: check caps argument. */
	caps->supportedAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

	caps->minSrcPosition.x = 0;
	caps->minSrcPosition.y = 0;
	caps->maxSrcPosition.x = max_w - 1;
	caps->maxSrcPosition.y = max_h - 1;

	caps->minSrcExtent.width = 1;
	caps->minSrcExtent.height = 1;
	caps->maxSrcExtent.width = max_w;
	caps->maxSrcExtent.height = max_h;

	caps->minDstPosition.x = 0;
	caps->minDstPosition.y = 0;
	caps->maxDstPosition.x = max_w - min_w;
	caps->maxDstPosition.y = max_h - min_h;

	caps->minDstExtent.width = min_w;
	caps->minDstExtent.height = min_h;
	caps->maxDstExtent.width = max_w;
	caps->maxDstExtent.height = max_h;

	return VK_SUCCESS;
}
