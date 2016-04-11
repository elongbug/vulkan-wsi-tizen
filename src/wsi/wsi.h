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

#ifndef WSI_H
#define WSI_H

#include <config.h>
#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <vulkan/vk_icd.h>
#include <utils.h>
#include <tpl.h>

typedef struct vk_surface	vk_surface_t;
typedef struct vk_swapchain	vk_swapchain_t;

struct vk_surface {
	union {
		VkIcdSurfaceBase	base;
		VkIcdSurfaceDisplay	display;

#ifdef VK_USE_PLATFORM_XLIB_KHR
		VkIcdSurfaceXlib	xlib;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
		VkIcdSurfaceXcb		xcb;
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
		VkIcdSurfaceWayland	wayland;
#endif
	} platform;

	VkAllocationCallbacks	allocator;

	struct {
		tpl_display_t	   *display;
		tpl_surface_t	   *surface;
	} tpl;
};

struct vk_swapchain {
	vk_surface_t		   *vk_surface;
	VkAllocationCallbacks	allocator;
	tbm_surface_h		   *buffers;
	int						buffer_index;
	int						buffer_count;
};

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *name);

const VkAllocationCallbacks *
vk_get_allocator(void *parent, const VkAllocationCallbacks *allocator);

void *
vk_alloc(const VkAllocationCallbacks *allocator, size_t size, VkSystemAllocationScope scope);

void *
vk_realloc(const VkAllocationCallbacks *allocator, void *mem, size_t size,
		   VkSystemAllocationScope scope);

void
vk_free(const VkAllocationCallbacks *allocator, void *mem);

/* Entry point proto types. */
VKAPI_ATTR void VKAPI_CALL
vk_DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
					 const VkAllocationCallbacks *allocator);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice pdev, uint32_t queue_family_index,
									  VkSurfaceKHR surface, VkBool32 *supported);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice pdev, VkSurfaceKHR surface,
										   VkSurfaceCapabilitiesKHR *caps);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice pdev, VkSurfaceKHR surface,
									  uint32_t *format_count, VkSurfaceFormatKHR *formats);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice pdev, VkSurfaceKHR surface,
										   uint32_t *mode_count, VkPresentModeKHR *modes);

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *info,
					  const VkAllocationCallbacks *allocator, VkSwapchainKHR *swapchain);

VKAPI_ATTR void VKAPI_CALL
vk_DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
					   const VkAllocationCallbacks *allocator);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *image_count,
						 VkImage *images);

VKAPI_ATTR VkResult VKAPI_CALL
vk_AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
					   VkSemaphore semaphore, VkFence fence, uint32_t *image_index);

VKAPI_ATTR VkResult VKAPI_CALL
vk_QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *info);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice pdev, uint32_t *prop_count,
										 VkDisplayPropertiesKHR *props);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice pdev, uint32_t *prop_count,
											  VkDisplayPlanePropertiesKHR *props);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice pdev, uint32_t plane_index,
									   uint32_t *display_count, VkDisplayKHR *displays);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayModePropertiesKHR(VkPhysicalDevice pdev, VkDisplayKHR display, uint32_t *prop_count,
							   VkDisplayModePropertiesKHR *props);

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateDisplayModeKHR(VkPhysicalDevice pdev, VkDisplayKHR display,
						const VkDisplayModeCreateInfoKHR *info,
						const VkAllocationCallbacks *allocator, VkDisplayModeKHR *mode);

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice pdev, VkDisplayModeKHR mode,
								  uint32_t plane_index, VkDisplayPlaneCapabilitiesKHR *caps);

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *info,
								const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchain_count,
							 const VkSwapchainCreateInfoKHR *infos,
							 const VkAllocationCallbacks *allocator, VkSwapchainKHR *swapchains);

#ifdef VK_USE_PLATFORM_XLIB_KHR
VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *info,
						const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice pdev, uint32_t queue_family_index,
											   Display *dpy, VisualID visual_id);
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *info,
					   const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice pdev, uint32_t queue_family_index,
											  xcb_connection_t *connection,
											  xcb_visualid_t visual_id);
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *info,
						   const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice pdev,
												  uint32_t queue_family_index,
												  struct wl_display *display);
#endif

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_GetInstanceProcAddr(VkInstance instance, const char *name);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_GetDeviceProcAddr(VkDevice device, const char *name);

#endif /* WSI_H */
