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
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

static vk_icd_t	icd;

vk_icd_t *
vk_get_icd(void)
{
	return &icd;
}

static const VkExtensionProperties wsi_instance_extensions[] = {
	{ VK_KHR_SURFACE_EXTENSION_NAME, 25 },
	{ VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, 4 },
};

static void __attribute__((constructor))
module_init(void)
{
	const char	*filename;
	uint32_t	count;
	VkResult	res;
	PFN_vkEnumerateInstanceExtensionProperties enum_inst_exts;

	/* Get env var for ICD path. */
	filename = getenv("VK_TIZEN_ICD");
	VK_CHECK(filename, return, "No ICD library given.\n");

	dlerror();

	/* Open ICD file. */
	icd.lib = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
	VK_CHECK(icd.lib, return, "dlopen() failed - %s\n", dlerror());

	/* Retrieve our first entry point for vulkan symbols. */
	icd.get_proc_addr = dlsym(icd.lib, "vk_icdGetInstanceProcAddr");
	VK_CHECK(icd.get_proc_addr, return, "vk_icdGetInstanceProcAddr() not present.\n");

	/* Dispatch device extension enumeration function. */
	icd.enum_dev_exts = (void *)icd.get_proc_addr(NULL, "vkEnumerateDeviceExtensionProperties");
	VK_CHECK(icd.enum_dev_exts, return, "vkEnumerateDeviceExtensionProperties() not present.\n");

	/* Retrieve WSI-ICD interface functions. */
	icd.create_presentable_image =
		(void *)icd.get_proc_addr(NULL,	"vkCreateImageFromNativeBufferTIZEN");

	/* Initialize instance extensions. */
	enum_inst_exts = (void *)icd.get_proc_addr(NULL, "vkEnumerateInstanceExtensionProperties");
	VK_CHECK(enum_inst_exts, return, "vkEnumerateInstanceExtensionProperties() not present.\n");

	res = enum_inst_exts(NULL, &count, NULL);
	VK_CHECK(res == VK_SUCCESS, return, "vkEnumerateInstanceExtensionProperties() failed.\n");

	count += ARRAY_LENGTH(wsi_instance_extensions);

	icd.instance_extensions = malloc(count * sizeof(VkExtensionProperties));
	VK_CHECK(icd.instance_extensions, return, "malloc() failed.\n");

	res = enum_inst_exts(NULL, &count, icd.instance_extensions);
	VK_CHECK(res == VK_SUCCESS, return, "vkEnumerateInstanceExtensionProperties() failed.\n");

	memcpy(icd.instance_extensions + count, wsi_instance_extensions,
		   ARRAY_LENGTH(wsi_instance_extensions) * sizeof(VkExtensionProperties));

	icd.instance_extension_count = count + ARRAY_LENGTH(wsi_instance_extensions);
}

static void __attribute__((destructor))
module_fini(void)
{
	if (icd.lib)
		dlclose(icd.lib);
}
