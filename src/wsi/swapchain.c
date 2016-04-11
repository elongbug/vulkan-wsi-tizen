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
vk_CreateSwapchainKHR(VkDevice							 device,
					  const VkSwapchainCreateInfoKHR	*info,
					  const VkAllocationCallbacks		*allocator,
					  VkSwapchainKHR					*swapchain)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_CreateSharedSwapchainsKHR(VkDevice						 device,
							 uint32_t						 swapchain_count,
							 const VkSwapchainCreateInfoKHR	*infos,
							 const VkAllocationCallbacks	*allocator,
							 VkSwapchainKHR					*swapchains)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_DestroySwapchainKHR(VkDevice						 device,
					   VkSwapchainKHR				 swapchain,
					   const VkAllocationCallbacks	*allocator)
{
	/* TODO: */
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_GetSwapchainImagesKHR(VkDevice		 device,
						 VkSwapchainKHR	 swapchain,
						 uint32_t		*image_count,
						 VkImage		*images)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_AcquireNextImageKHR(VkDevice			 device,
					   VkSwapchainKHR	 swapchain,
					   uint64_t			 timeout,
					   VkSemaphore		 semaphore,
					   VkFence			 fence,
					   uint32_t			*image_index)
{
	/* TODO: */
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_QueuePresentKHR(VkQueue					 queue,
				   const VkPresentInfoKHR	*info)
{
	/* TODO: */
	return VK_SUCCESS;
}
