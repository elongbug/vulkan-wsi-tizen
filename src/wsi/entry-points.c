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
#include <stdlib.h>
#include <dlfcn.h>

#define VK_ENTRY_POINT(name, type) { "vk"#name, vk_##name, VK_FUNC_TYPE_##type }

typedef enum vk_func_type		vk_func_type_t;
typedef struct vk_entry			vk_entry_t;
typedef struct vk_icd_loader	vk_icd_loader_t;

enum vk_func_type {
	VK_FUNC_TYPE_GLOBAL,
	VK_FUNC_TYPE_INSTANCE,
	VK_FUNC_TYPE_DEVICE,
};

struct vk_entry {
	const char		*name;
	void			*func;
	vk_func_type_t	 type;
};

struct vk_icd_loader {
	void						*lib;
	PFN_vkGetInstanceProcAddr	 gpa;
};

static const vk_entry_t	entry_points[] = {
	VK_ENTRY_POINT(DestroySurfaceKHR, INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceSurfaceSupportKHR, INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceSurfaceCapabilitiesKHR, INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceSurfaceFormatsKHR, INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceSurfacePresentModesKHR, INSTANCE),
	VK_ENTRY_POINT(CreateSwapchainKHR, DEVICE),
	VK_ENTRY_POINT(DestroySwapchainKHR, DEVICE),
	VK_ENTRY_POINT(GetSwapchainImagesKHR, DEVICE),
	VK_ENTRY_POINT(AcquireNextImageKHR, DEVICE),
	VK_ENTRY_POINT(QueuePresentKHR, DEVICE),
	VK_ENTRY_POINT(GetPhysicalDeviceDisplayPropertiesKHR, INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceDisplayPlanePropertiesKHR, INSTANCE),
	VK_ENTRY_POINT(GetDisplayPlaneSupportedDisplaysKHR, INSTANCE),
	VK_ENTRY_POINT(GetDisplayModePropertiesKHR, INSTANCE),
	VK_ENTRY_POINT(CreateDisplayModeKHR, INSTANCE),
	VK_ENTRY_POINT(GetDisplayPlaneCapabilitiesKHR, INSTANCE),
	VK_ENTRY_POINT(CreateDisplayPlaneSurfaceKHR, INSTANCE),
	VK_ENTRY_POINT(CreateSharedSwapchainsKHR, DEVICE),

#ifdef VK_USE_PLATFORM_XLIB_KHR
	VK_ENTRY_POINT(CreateXlibSurfaceKHR,INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceXlibPresentationSupportKHR,INSTANCE),
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
	VK_ENTRY_POINT(CreateXcbSurfaceKHR,INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceXcbPresentationSupportKHR,INSTANCE),
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	VK_ENTRY_POINT(CreateWaylandSurfaceKHR,INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceWaylandPresentationSupportKHR,INSTANCE),
#endif

	VK_ENTRY_POINT(GetInstanceProcAddr, INSTANCE),
	VK_ENTRY_POINT(GetDeviceProcAddr, DEVICE),
};

static const vk_entry_t *
get_entry_point(const char *name)
{
	/* TODO: Apply perfect hashing. */

	uint32_t i;

	for (i = 0; i < ARRAY_LENGTH(entry_points); ++i) {
		if (strcmp(entry_points[i].name, name) == 0)
			return &entry_points[i];
	}

	return NULL;
}

static vk_icd_loader_t	icd;

static void __attribute__((constructor))
icd_init(void)
{
	const char *filename;

	filename = getenv("VK_TIZEN_ICD");
	VK_CHECK(filename, return, "No ICD library given.\n");

	dlerror();

	icd.lib = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
	VK_CHECK(icd.lib, return, "dlopen() failed - %s\n", dlerror());

	icd.gpa = dlsym(icd.lib, "vk_icdGetInstanceProcAddr");
	VK_CHECK(icd.gpa, return, "vk_icdGetInstanceProcAddr() not present.\n");
}

static void __attribute__((destructor))
icd_fini(void)
{
	if (icd.lib)
		dlclose(icd.lib);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_GetInstanceProcAddr(VkInstance instance, const char *name)
{
	const vk_entry_t			*entry = get_entry_point(name);
	PFN_vkGetInstanceProcAddr	 gipa;

	/* According to vulkan specification 1.0, when instance is NULL, name must be one of global
	 * functions. When instance is not NULL, then name must be not one of global functions. */
	if (entry) {
		if (instance == NULL && entry->type == VK_FUNC_TYPE_GLOBAL)
			return entry->func;

		if (instance != NULL && entry->type != VK_FUNC_TYPE_GLOBAL)
			return entry->func;

		if (entry->func == vk_GetInstanceProcAddr)
			return entry->func;

		if (entry->func == vk_GetDeviceProcAddr)
			return entry->func;

		return NULL;
	}

	/* TODO: Avoid getting GIPA on the fly. */
	gipa = (PFN_vkGetInstanceProcAddr)icd.gpa(instance, "vkGetInstanceProcAddr");

	return gipa(instance, name);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_GetDeviceProcAddr(VkDevice device, const char *name)
{
	const vk_entry_t		*entry = get_entry_point(name);
	PFN_vkGetDeviceProcAddr	 gdpa;

	if (device == NULL)
		return NULL;

	if (entry) {
		if ( entry->type == VK_FUNC_TYPE_DEVICE)
			return entry->func;

		return NULL;
	}

	/* TODO: We are trying to get the most specific device functions here. */
	gdpa = (PFN_vkGetDeviceProcAddr)icd.gpa(NULL, "vkGetDeviceProcAddr");
	gdpa = (PFN_vkGetDeviceProcAddr)gdpa(device, "vkGetDeviceProcAddr");

	return gdpa(device, name);

}

VK_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *name)
{
	const vk_entry_t *entry = get_entry_point(name);

	if (entry)
		return entry->func;

	return icd.gpa(instance, name);
}