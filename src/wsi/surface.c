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

VKAPI_ATTR void VKAPI_CALL
vk_DestroySurfaceKHR(VkInstance						 instance,
					 VkSurfaceKHR					 surface,
					 const VkAllocationCallbacks	*allocator)
{
	/* TODO: */
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateXlibSurfaceKHR(VkInstance							 instance,
						const VkXlibSurfaceCreateInfoKHR	*info,
						const VkAllocationCallbacks			*allocator,
						VkSurfaceKHR						*surface)
{
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
	/* TODO: */
	return VK_SUCCESS;
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
vk_GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice	 pdev,
									  uint32_t			 queue_family_index,
									  VkSurfaceKHR		 surface,
									  VkBool32			*supported)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice			 pdev,
										   VkSurfaceKHR				 surface,
										   VkSurfaceCapabilitiesKHR	*caps)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice		 pdev,
									  VkSurfaceKHR			 surface,
									  uint32_t				*format_count,
									  VkSurfaceFormatKHR	*formats)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice	 pdev,
										   VkSurfaceKHR		 surface,
										   uint32_t			*mode_count,
										   VkPresentModeKHR	*modes)
{
	/* TODO: */
	return VK_SUCCESS;
}
