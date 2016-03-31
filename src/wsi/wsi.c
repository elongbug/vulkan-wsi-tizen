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

static VKAPI_ATTR void VKAPI_CALL
destroy_surface_khr(VkInstance						 instance,
					 VkSurfaceKHR					 surface,
					 const VkAllocationCallbacks	*allocator)
{
	/* TODO: */
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_physical_device_surface_support_khr(VkPhysicalDevice	 pdev,
										uint32_t			 queue_family_index,
										VkSurfaceKHR		 surface,
										VkBool32			*supported)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_physical_device_surface_capabilities_khr(VkPhysicalDevice			 pdev,
											 VkSurfaceKHR				 surface,
											 VkSurfaceCapabilitiesKHR	*caps)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_physical_device_surface_formats_khr(VkPhysicalDevice	 pdev,
										VkSurfaceKHR		 surface,
										uint32_t			*format_count,
										VkSurfaceFormatKHR	*formats)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_physical_device_surface_present_modes_khr(VkPhysicalDevice	 pdev,
											  VkSurfaceKHR		 surface,
											  uint32_t			*mode_count,
											  VkPresentModeKHR	*modes)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_swapchain_khr(VkDevice						 dev,
					 const VkSwapchainCreateInfoKHR	*info,
					 const VkAllocationCallbacks	*allocator,
					 VkSwapchainKHR					*swapchain)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_swapchain_khr(VkDevice						 dev,
					  VkSwapchainKHR				 swapchain,
					  const VkAllocationCallbacks	*allocator)
{
	/* TODO: */
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_swapchain_images_khr(VkDevice			 dev,
						 VkSwapchainKHR		 swapchain,
						 uint32_t			*image_count,
						 VkImage			*images)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
acquire_next_image_khr(VkDevice			 device,
					   VkSwapchainKHR	 swapchain,
					   uint64_t			 timeout,
					   VkSemaphore		 semaphore,
					   VkFence			 fence,
					   uint32_t			*image_index)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
queue_present_khr(VkQueue					 queue,
				  const VkPresentInfoKHR	*info)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_physical_device_display_properties_khr(VkPhysicalDevice			 pdev,
										   uint32_t					*prop_count,
										   VkDisplayPropertiesKHR	*props)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_physical_device_display_plane_properties_khr(VkPhysicalDevice				 pdev,
												 uint32_t						*prop_count,
												 VkDisplayPlanePropertiesKHR	*props)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_display_plane_supported_displays_khr(VkPhysicalDevice	 pdev,
										 uint32_t			 plane_index,
										 uint32_t			*display_count,
										 VkDisplayKHR		*displays)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_display_mode_properties_khr(VkPhysicalDevice			 pdev,
								VkDisplayKHR				 display,
								uint32_t					*prop_count,
								VkDisplayModePropertiesKHR	*props)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_display_mode_khr(VkPhysicalDevice					 pdev,
						VkDisplayKHR						 display,
						const VkDisplayModeCreateInfoKHR	*info,
						const VkAllocationCallbacks			*allocator,
						VkDisplayModeKHR					*mode)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_display_plane_capabilities_khr(VkPhysicalDevice					 pdev,
								   VkDisplayModeKHR					 mode,
								   uint32_t							 plane_index,
								   VkDisplayPlaneCapabilitiesKHR	*caps)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_display_plane_surface_khr(VkInstance								instance,
								 const VkDisplaySurfaceCreateInfoKHR	*info,
								 const VkAllocationCallbacks			*allocator,
								 VkSurfaceKHR							*surface)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_shared_swapchains_khr(VkDevice						 device,
							 uint32_t						 swapchain_count,
							 const VkSwapchainCreateInfoKHR	*infos,
							 const VkAllocationCallbacks	*allocator,
							 VkSwapchainKHR					*swapchains)
{
	/* TODO: */
	return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
static VKAPI_ATTR VkResult VKAPI_CALL
create_xlib_surface_khr(VkInstance							instance,
						const VkXlibSurfaceCreateInfoKHR	*info,
						const VkAllocationCallbacks			*allocator,
						VkSurfaceKHR						*surface)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
get_physical_device_xlib_presentation_support_khr(VkPhysicalDevice	 pdev,
												  uint32_t			 queue_family_index,
												  Display			*dpy,
												  VisualID			 visual_id)
{
	/* TODO: */
	return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
static VKAPI_ATTR VkResult VKAPI_CALL
create_xcb_surface_khr(VkInstance						 instance,
					   const VkXcbSurfaceCreateInfoKHR	*info,
					   const VkAllocationCallbacks		*allocator,
					   VkSurfaceKHR						*surface)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
get_physical_device_xcb_presentation_support_khr(VkPhysicalDevice	 pdev,
												 uint32_t			 queue_family_index,
												 xcb_connection_t	*connection,
												 xcb_visualid_t		 visual_id)
{
	/* TODO: */
	return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
static VKAPI_ATTR VkResult VKAPI_CALL
create_wayland_surface_khr(VkInstance							 instance,
						   const VkWaylandSurfaceCreateInfoKHR	*info,
						   const VkAllocationCallbacks			*allocator,
						   VkSurfaceKHR							*surface)
{
	/* TODO: */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
get_physical_device_wayland_presentation_support_khr(VkPhysicalDevice	 pdev,
													 uint32_t			 queue_family_index,
													 struct wl_display	*display)
{
	/* TODO: */
	return VK_TRUE;
}
#endif
