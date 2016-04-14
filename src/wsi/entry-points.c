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

	PFN_vkEnumerateInstanceExtensionProperties	enum_instance_extensions;
	PFN_vkEnumerateDeviceExtensionProperties	enum_device_extensions;

	uint32_t				 global_extension_count;
	VkExtensionProperties	*global_extensions;
};

static const vk_entry_t	entry_points[] = {
	VK_ENTRY_POINT(DestroySurfaceKHR, INSTANCE),
	VK_ENTRY_POINT(EnumerateInstanceExtensionProperties, INSTANCE),
	VK_ENTRY_POINT(EnumerateDeviceExtensionProperties, INSTANCE),
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
	VK_ENTRY_POINT(CreateWaylandSurfaceKHR,INSTANCE),
	VK_ENTRY_POINT(GetPhysicalDeviceWaylandPresentationSupportKHR,INSTANCE),
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

static const VkExtensionProperties global_extensions[] = {
	{ VK_KHR_SURFACE_EXTENSION_NAME, 25 },
	{ VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, 4 },
};

static void __attribute__((constructor))
icd_init(void)
{
	const char *filename;
	VkResult	res;
	uint32_t	count;

	filename = getenv("VK_TIZEN_ICD");
	VK_CHECK(filename, return, "No ICD library given.\n");

	dlerror();

	icd.lib = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
	VK_CHECK(icd.lib, return, "dlopen() failed - %s\n", dlerror());

	icd.gpa = dlsym(icd.lib, "vk_icdGetInstanceProcAddr");
	VK_CHECK(icd.gpa, return, "vk_icdGetInstanceProcAddr() not present.\n");

	/* Retrieve extension enumeration functions. */
	icd.enum_instance_extensions = (void *)icd.gpa(NULL, "vkEnumerateInstanceExtensionProperties");
	VK_CHECK(icd.enum_instance_extensions, return,
			 "vkEnumerateInstanceExtensionProperties() not present.\n");

	icd.enum_device_extensions = (void *)icd.gpa(NULL, "vkEnumerateDeviceExtensionProperties");
	VK_CHECK(icd.enum_device_extensions, return,
			 "vkEnumerateDeviceExtensionProperties() not present.\n");

	/* Get ICD global extension count. */
	res = icd.enum_instance_extensions(NULL, &count, NULL);
	VK_CHECK(res == VK_SUCCESS, return, "vkEnumerateInstanceExtensionProperties() failed.\n");

	/* Allocate memory to hold the extensions both for ICD and WSI. */
	icd.global_extensions =
		malloc((count + ARRAY_LENGTH(global_extensions)) * sizeof(VkExtensionProperties));
	VK_CHECK(icd.global_extensions, return, "malloc() failed.\n");

	/* Copy ICD extensions first. */
	res = icd.enum_instance_extensions(NULL, &count, icd.global_extensions);
	VK_CHECK(res == VK_SUCCESS, return, "vkEnumerateInstanceExtensionProperties() failed.\n");

	/* Append WSI extensions. */
	memcpy(icd.global_extensions + count, global_extensions,
		   ARRAY_LENGTH(global_extensions) * sizeof(VkExtensionProperties));

	icd.global_extension_count = count + ARRAY_LENGTH(global_extensions);
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

VKAPI_ATTR VkResult VKAPI_CALL
vk_EnumerateInstanceExtensionProperties(const char				*layer_name,
										uint32_t				*count,
										VkExtensionProperties	*extensions)
{
	if (!extensions) {
		*count = icd.global_extension_count;
		return VK_SUCCESS;
	}

	*count = MIN(*count, icd.global_extension_count);
	memcpy(extensions, icd.global_extensions, *count * sizeof(VkExtensionProperties));

	if (*count < icd.global_extension_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

static const VkExtensionProperties device_extensions[] = {
	{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, 67 },
};

VKAPI_ATTR VkResult VKAPI_CALL
vk_EnumerateDeviceExtensionProperties(VkPhysicalDevice		 pdev,
									  const char			*layer_name,
									  uint32_t				*count,
									  VkExtensionProperties	*extensions)
{
	uint32_t				 ext_count, copied = 0;
	uint32_t				 icd_count, max_count;
	VkResult				 result;

	result = icd.enum_device_extensions(pdev, layer_name, &icd_count, NULL);
	VK_CHECK(result == VK_SUCCESS, return VK_ERROR_OUT_OF_HOST_MEMORY,
			 "vkEnumerateDeviceExtensionProperties() failed.\n");

	max_count = icd_count + ARRAY_LENGTH(device_extensions);

	if (!extensions) {
		/* Just return the number of enabled extension properties in this case. */
		*count = max_count;
		return VK_SUCCESS;
	}

	/* We should copy ICD extensions and WSI extensions together into the given pointer. */

	ext_count = MIN(*count, icd_count);
	result = icd.enum_device_extensions(pdev, layer_name, &ext_count, extensions);
	VK_CHECK(result == VK_SUCCESS || result == VK_INCOMPLETE, return VK_ERROR_OUT_OF_HOST_MEMORY,
			 "vkEnumerateDeviceExtensionProperties() failed.\n");

	/* Advance the destination pointer. */
	extensions += ext_count;
	copied += ext_count;

	/* Calculate remaining extensions to copy. */
	ext_count = *count - ext_count;

	if (ext_count > 0) {
		ext_count = MIN(ext_count, ARRAY_LENGTH(device_extensions));
		memcpy(extensions, device_extensions, ext_count * sizeof(VkExtensionProperties));
		copied += ext_count;
	}

	*count = copied;

	if (*count < max_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}
