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

/**
 * This header file defines interface between tizen vulkan WSI implementation and vendor vulkan ICD.
 * Following is a brief diagram how vulkan is provided on tizen platform.
 *
 *      -------------------------------------------------
 *      |             Vulkan Application                |
 *      -------------------------------------------------
 *      -------------------------------------------------
 *      |            Khronos Vulkan Loader              |
 *      -------------------------------------------------
 *      -------------------------------------------------
 *      |              Tizen Vulkan WSI                 |
 *      -------------------------------------------------
 *      -------------------------------------------------
 *      |                Vendor ICD                     |
 *      -------------------------------------------------
 *
 * Khronos vulkan loader provides libvulkan.so so that applications can link against it to use
 * vulkan symbols. Application can choose layers to activate and ICD to load by configuring khronos
 * vulkan loader settings.
 *
 * On tizen, tizen vulkan WSI wraps a vendor ICD and behaves as if it is a complete vulkan ICD. As
 * we all know, loader will dispatch vulkan functions using vk_icdGetInstanceProcAddr(). WSI
 * provides the function to the loader and the function dispatches WSI functions and vendor ICD
 * functions.
 *
 * Vendor ICD does not need to concern the above loading mechanism. They can implement their ICD
 * like a normal khronos-loader-loadable ICD. However, in order to implement WSI functions, vendor
 * ICD should provides some functions to WSI. The functions are defined in this header file and
 * they should be exposed in the vendor ICD .so file so that it can be retrieved via dlsym.
 */
#ifndef VULKAN_WSI_TIZEN_H
#define VULKAN_WSI_TIZEN_H

#include <vulkan/vulkan.h>
#include <tpl.h>

/**
 * Create a VkImage which is used for presentation from the given tbm_surface_h.
 *
 * @param device	VkDevice where the image belongs to.
 * @param info		Pointer to the structure VkImageCreateInfo, see below descriptions.
 * @param buffer	tbm_surface_h which is the actual pixel storage of the image.
 *
 * After an application creates a swapchain, swapchain images can be queried via
 * vkGetSwapchainImagesKHR(). However, VkImage is vendor ICD defined object, so vendor ICDs should
 * provide functions for creating VkImage from native buffer. On tizen, we use tbm_surface_h as
 * the native buffer type.
 *
 * Vendors can get various information about the tbm_surface_h using libtbm like dimension, format,
 * planes (multi-planar) including tbm_bo. Vendor can implement their ICD using the provided tbm
 * APIs.
 *
 * Paremeter "info" should be NULL. It is reserved for future use.
 */
VkImage
vk_create_presentable_image(VkDevice device, const VkImageCreateInfo *info,	tbm_surface_h buffer);

/**
 * Turn the given semaphore into signaled state.
 *
 * @param	semaphore	VkSemaphore to signal.
 *
 * @returns	VK_TRUE if succeed, VK_FALSE otherwise.
 *
 * vkAcquireNextImageKHR() gives semaphore or fence to be signaled when the acquired image is ready
 * to be accessed. When the image will be ready is WSI implementation specific and so WSI should be
 * able to signal the given semaphore when the image is ready.
 *
 * This function should be thread-safe. WSI should be able to call this function from any thread.
 */
VkBool32
vk_signal_semaphore(VkSemaphore semaphore);

/**
 * Wait for given semaphores to be signaled.
 *
 * @param count			the number of semaphores to wait for.
 * @param semaphores	pointer to the array of semaphores.
 *
 * @returns	VK_TRUE if succeed, VK_FALSE otherwise.
 *
 * WSI should wait for semaphores before issueing present request to the presentation engine in
 * vkQueuepresentKHR(). This function returns when all of the given semaphores are signaled.
 */
VkBool32
vk_wait_for_semaphores(uint32_t count, const VkSemaphore *semaphores);

/**
 * Turn the given fence into signaled state.
 *
 * @param	fence	VkFence to signal.
 *
 * @returns	VK_TRUE if succeed, VK_FALSE otherwise.
 *
 * @see vk_signal_semaphore()
 */
VkBool32
vk_signal_fence(VkFence fence);

#endif /* VULKAN_WSI_TIZEN_H */
