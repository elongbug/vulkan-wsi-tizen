#ifndef VULKAN_WSI_TIZEN_H
#define VULKAN_WSI_TIZEN_H

#include <stdbool.h>
#include <vulkan/vk_icd.h>
#include <tpl.h>

VkImage
vk_create_presentable_image(VkDevice device, const VkImageCreateInfo *info,	tbm_surface_h buffer);

#endif /* VULKAN_WSI_TIZEN_H */
