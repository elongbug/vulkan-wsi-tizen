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
	vk_surface_t *sfc = (vk_surface_t *)surface;

	/* TODO: */

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

	sfc->allocator = *allocator;
	sfc->platform.base.platform = VK_ICD_WSI_PLATFORM_WAYLAND;
	sfc->platform.wayland.display = info->display;
	sfc->platform.wayland.surface = info->surface;

	*surface = (VkSurfaceKHR)sfc;

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
