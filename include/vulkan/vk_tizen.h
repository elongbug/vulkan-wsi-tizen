#ifndef VK_TIZEN_H
#define VK_TIZEN_H

#include <vulkan/vulkan.h>
#include <tbm_surface.h>

typedef VkResult (VKAPI_PTR *PFN_vkCreateImageFromNativeBufferTIZEN)(VkDevice device, tbm_surface_h
																	 surface, const
																	 VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImage *pImage);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageFromNativeBufferTIZEN(
	VkDevice									device,
	tbm_surface_h								surface,
	const VkImageCreateInfo *					pCreateInfo,
	const VkAllocationCallbacks *				pAllocator,
	VkImage *									pImage);
#endif

#endif /* VK_TIZEN_H */
