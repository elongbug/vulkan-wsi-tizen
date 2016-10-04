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

void
vk_display_init(vk_physical_device_t *pdev)
{
	/* TODO: */

	pdev->display_count = 0;
	pdev->displays = NULL;

	pdev->plane_count = 0;
	pdev->planes = NULL;
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
	memset(caps, 0x00, sizeof(VkDisplayPlaneCapabilitiesKHR));

	/* TODO: Fill in the caps argument. */

	return VK_SUCCESS;
}
