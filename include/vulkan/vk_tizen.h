#ifndef VK_TIZEN_H
#define VK_TIZEN_H

#include <vulkan/vulkan.h>
#include <tbm_surface.h>

typedef VkResult (VKAPI_PTR *PFN_vkCreateImageFromNativeBufferTIZEN)(VkDevice device, tbm_surface_h
																	 surface, const
																	 VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImage *pImage);

typedef VkResult (VKAPI_PTR *PFN_vkQueueSignalReleaseImageTIZEN)
	(VkQueue queue, uint32_t waitSemaphoreCount,
	 const VkSemaphore *pWaitSemaphores,
	 VkImage image, int *pNativeFenceFd);
typedef VkResult (VKAPI_PTR *PFN_vkAcquireImageTIZEN)
	(VkDevice device, VkImage image, int nativeFenceFd,
	 VkSemaphore semaphore, VkFence fence);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageFromNativeBufferTIZEN(
	VkDevice									device,
	tbm_surface_h								surface,
	const VkImageCreateInfo *					pCreateInfo,
	const VkAllocationCallbacks *				pAllocator,
	VkImage *									pImage);
VKAPI_PTR VkResult vkQueueSignalReleaseImageTIZEN(VkQueue queue, uint32_t waitSemaphoreCount,
												  const VkSemaphore *pWaitSemaphores,
												  VkImage image, int *pNativeFenceFd);
VKAPI_PTR VkResult vkAcquireImageTIZEN(VkDevice device, VkImage image, int nativeFenceFd,
									   VkSemaphore semaphore, VkFence fence);

#endif

#endif /* VK_TIZEN_H */
