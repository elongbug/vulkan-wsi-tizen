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

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice		 pdev,
										 uint32_t				*prop_count,
										 VkDisplayPropertiesKHR	*props)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice				 pdev,
											  uint32_t						*prop_count,
											  VkDisplayPlanePropertiesKHR	*props)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice	 pdev,
									   uint32_t			 plane_index,
									   uint32_t			*display_count,
									   VkDisplayKHR		*displays)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayModePropertiesKHR(VkPhysicalDevice				 pdev,
							   VkDisplayKHR					 display,
							   uint32_t						*prop_count,
							   VkDisplayModePropertiesKHR	*props)
{
	/* TODO: */
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
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice				 pdev,
								  VkDisplayModeKHR				 mode,
								  uint32_t						 plane_index,
								  VkDisplayPlaneCapabilitiesKHR	*caps)
{
	/* TODO: */
	return VK_SUCCESS;
}
