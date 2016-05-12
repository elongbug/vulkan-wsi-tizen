#ifndef VULKAN_WSI_TIZEN_H
#define VULKAN_WSI_TIZEN_H

#include <vulkan/vulkan.h>
#include <tpl.h>

VkImage
vk_create_presentable_image(VkDevice device, const VkImageCreateInfo *info,	tbm_surface_h buffer);

VkBool32
vk_signal_semaphore(VkSemaphore semaphore);

VkBool32
vk_wait_for_semaphores(uint32_t count, const VkSemaphore *semaphores);

VkBool32
vk_signal_fence(VkFence fence);

#endif /* VULKAN_WSI_TIZEN_H */
