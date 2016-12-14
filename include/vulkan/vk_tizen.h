#ifndef VK_TIZEN_H
#define VK_TIZEN_H

#include <vulkan/vulkan.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <tbm_bufmgr.h>

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
typedef VkResult (VKAPI_PTR *PFN_vkCreateTBMQueueSurfaceKHR)
	(VkInstance instance,
	 const tbm_bufmgr bufmgr,
	 const tbm_surface_queue_h queue,
	 const VkAllocationCallbacks *pAllocator,
	 VkSurfaceKHR *surface);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageFromNativeBufferTIZEN(
	VkDevice									device,
	tbm_surface_h								surface,
	const VkImageCreateInfo *					pCreateInfo,
	const VkAllocationCallbacks *				pAllocator,
	VkImage *									pImage);
VKAPI_ATTR VkResult VKAPI_CALL vkQueueSignalReleaseImageTIZEN(VkQueue queue,
															  uint32_t waitSemaphoreCount,
															  const VkSemaphore *pWaitSemaphores,
															  VkImage image, int *pNativeFenceFd);
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireImageTIZEN(VkDevice device, VkImage image,
												   int nativeFenceFd,
												   VkSemaphore semaphore, VkFence fence);
VKAPI_ATTR VkResult VKAPI_CALL vkCreateTBMQueueSurfaceKHR(VkInstance instance,
														  const tbm_bufmgr bufmgr,
														  const tbm_surface_queue_h queue,
														  const VkAllocationCallbacks *pAllocator,
														  VkSurfaceKHR *surface);
#endif

#define VK_ICD_WSI_PLATFORM_TBM_QUEUE 0x0000000F

#endif /* VK_TIZEN_H */
