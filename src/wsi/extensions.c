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

VKAPI_ATTR VkResult VKAPI_CALL
vk_EnumerateInstanceExtensionProperties(const char				*layer_name,
										uint32_t				*count,
										VkExtensionProperties	*extensions)
{
	vk_icd_t *icd = vk_get_icd();

	if (!extensions) {
		*count = icd->instance_extension_count;
		return VK_SUCCESS;
	}

	*count = MIN(*count, icd->instance_extension_count);
	memcpy(extensions, icd->instance_extensions, *count * sizeof(VkExtensionProperties));

	if (*count < icd->instance_extension_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

static const VkExtensionProperties wsi_device_extensions[] = {
	{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, 67 },
};

VKAPI_ATTR VkResult VKAPI_CALL
vk_EnumerateDeviceExtensionProperties(VkPhysicalDevice		 pdev,
									  const char			*layer_name,
									  uint32_t				*count,
									  VkExtensionProperties	*extensions)
{
	vk_icd_t	*icd = vk_get_icd();
	uint32_t	 max_ext_count, remaining, copied = 0;
	VkResult	 result;

	result = icd->enum_dev_exts(pdev, layer_name, &max_ext_count, NULL);
	VK_CHECK(result == VK_SUCCESS, return VK_ERROR_OUT_OF_HOST_MEMORY,
			 "vkEnumerateDeviceExtensionProperties() failed.\n");

	max_ext_count += ARRAY_LENGTH(wsi_device_extensions);

	if (!extensions) {
		*count = max_ext_count;
		return VK_SUCCESS;
	}

	/* Copy ICD extensions and WSI extensions together into the given pointer. */

	/* Calculate the number of extensions we have to copy. */
	remaining = MIN(*count, max_ext_count);

	/* Copy ICD extensions first. */
	copied = remaining;
	result = icd->enum_dev_exts(pdev, layer_name, &copied, extensions);
	VK_CHECK(result == VK_SUCCESS || result == VK_INCOMPLETE, return VK_ERROR_OUT_OF_HOST_MEMORY,
			 "vkEnumerateDeviceExtensionProperties() failed.\n");

	/* Calculate remaining extensions to copy. */
	remaining = MIN(remaining - copied, ARRAY_LENGTH(wsi_device_extensions));
	memcpy(extensions + copied, wsi_device_extensions, remaining * sizeof(VkExtensionProperties));
	copied += remaining;

	/* Return the number of extensions copied. */
	*count = copied;

	if (*count < max_ext_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}
