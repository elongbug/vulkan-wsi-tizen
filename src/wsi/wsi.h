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
#include <vulkan/vk_tizen.h>
#include <stdbool.h>
#include <vulkan/vk_icd.h>
#include <utils.h>
#include <tpl.h>

typedef struct vk_surface	vk_surface_t;
typedef struct vk_swapchain	vk_swapchain_t;
typedef struct vk_buffer	vk_buffer_t;
typedef struct vk_icd		vk_icd_t;

struct vk_icd {
	void	*lib;

	PFN_vkGetInstanceProcAddr	 				get_proc_addr;
	PFN_vkEnumerateDeviceExtensionProperties	enum_dev_exts;

	uint32_t				 instance_extension_count;
	VkExtensionProperties	*instance_extensions;

	/* WSI-ICD interface. */
	PFN_vkCreateImageFromNativeBufferTIZEN	create_presentable_image;
	PFN_vkQueueSignalReleaseImageTIZEN		queue_signal_release_image;
	PFN_vkAcquireImageTIZEN					acquire_image;
};

vk_icd_t *
vk_get_icd(void);

struct vk_buffer {
	tbm_surface_h	tbm;
	VkImage			image;
};

struct vk_swapchain {
	VkAllocationCallbacks	 allocator;
	VkSurfaceKHR			 surface;

	tpl_display_t			*tpl_display;
	tpl_surface_t			*tpl_surface;

	uint32_t				 buffer_count;
	vk_buffer_t				*buffers;
};

const VkAllocationCallbacks *
vk_get_allocator(void *parent, const VkAllocationCallbacks *allocator);

void *
vk_alloc(const VkAllocationCallbacks *allocator, size_t size, VkSystemAllocationScope scope);

void *
vk_realloc(const VkAllocationCallbacks *allocator, void *mem, size_t size,
		   VkSystemAllocationScope scope);

void
vk_free(const VkAllocationCallbacks *allocator, void *mem);

static inline tpl_display_t *
vk_get_tpl_display(tpl_handle_t native_dpy)
{
	tpl_display_t *display = tpl_display_create(TPL_BACKEND_WAYLAND_VULKAN_WSI, native_dpy);
	if (display == NULL) {
		display = tpl_display_get(native_dpy);
		tpl_object_reference((tpl_object_t *)display);
	}
	return display;
};

/* Entry point proto types. */
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *name);

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
vk_CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchain_count,
							 const VkSwapchainCreateInfoKHR *infos,
							 const VkAllocationCallbacks *allocator, VkSwapchainKHR *swapchains);

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice pdev,
												  uint32_t queue_family_index,
												  struct wl_display *display);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_GetInstanceProcAddr(VkInstance instance, const char *name);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_GetDeviceProcAddr(VkDevice device, const char *name);

VKAPI_ATTR VkResult VKAPI_CALL
vk_EnumerateInstanceExtensionProperties(const char *layer_name, uint32_t *count,
										VkExtensionProperties *extensions);

VKAPI_ATTR VkResult VKAPI_CALL
vk_EnumerateDeviceExtensionProperties(VkPhysicalDevice pdev, const char *layer_name,
									  uint32_t *count, VkExtensionProperties *extensions);

#endif /* WSI_H */
