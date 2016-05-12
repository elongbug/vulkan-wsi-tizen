/*
 * Copyright (C) 2015 Valve Corporation
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
 *
 * Author: Cody Northrop <cody@lunarg.com>
 * Author: David Pinedo <david@lunarg.com>
 * Author: Ian Elliott <ian@LunarG.com>
 * Author: Tony Barbour <tony@LunarG.com>
 */

#include "null-driver.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vk_icd.h>
#include <utils.h>

#if 0
#include <stdio.h>
#define NULLDRV_LOG_FUNC \
	do { \
		fflush(stdout); \
		fflush(stderr); \
		printf("null driver: %s\n", __FUNCTION__); \
		fflush(stdout); \
	} while (0)
#else
#define NULLDRV_LOG_FUNC do { } while (0)
#endif

struct nulldrv_base {
    void *loader_data;
    uint32_t magic;
    VkResult (*get_memory_requirements)(struct nulldrv_base *base,
										VkMemoryRequirements *mem_requirements);
};

struct nulldrv_obj {
    struct nulldrv_base base;
};

enum nulldrv_ext_type {
   NULLDRV_EXT_KHR_SWAPCHAIN,
   NULLDRV_EXT_COUNT,
   NULLDRV_EXT_INVALID = NULLDRV_EXT_COUNT,
};

struct nulldrv_instance {
    struct nulldrv_obj obj;
};

struct nulldrv_gpu {
    void *loader_data;
};

struct nulldrv_dev {
     struct nulldrv_base base;
     bool exts[NULLDRV_EXT_COUNT];
     struct nulldrv_desc_ooxx *desc_ooxx;
     struct nulldrv_queue *queues[1];
};

struct nulldrv_desc_ooxx {
    uint32_t surface_desc_size;
    uint32_t sampler_desc_size;
};

struct nulldrv_queue {
    struct nulldrv_base base;
    struct nulldrv_dev *dev;
};

struct nulldrv_rt_view {
    struct nulldrv_obj obj;
};

struct nulldrv_fence {
    struct nulldrv_obj obj;
};

struct nulldrv_img {
    struct nulldrv_obj obj;
    VkImageType type;
    int32_t depth;
    uint32_t mip_levels;
    uint32_t array_size;
    VkFlags usage;
    VkSampleCountFlagBits samples;
    size_t total_size;
	tbm_surface_h tbm_surface;
};

struct nulldrv_mem {
    struct nulldrv_base base;
    struct nulldrv_bo *bo;
    VkDeviceSize size;
};

struct nulldrv_sampler {
    struct nulldrv_obj obj;
};

struct nulldrv_shader_module {
    struct nulldrv_obj obj;
};

struct nulldrv_pipeline_cache {
    struct nulldrv_obj obj;
};

struct nulldrv_img_view {
    struct nulldrv_obj obj;
    struct nulldrv_img *img;
    float min_lod;
    uint32_t cmd_len;
};

struct nulldrv_buf {
    struct nulldrv_obj obj;
    VkDeviceSize size;
    VkFlags usage;
};

struct nulldrv_desc_layout {
    struct nulldrv_obj obj;
};

struct nulldrv_pipeline_layout {
    struct nulldrv_obj obj;
};

struct nulldrv_shader {
    struct nulldrv_obj obj;

};

struct nulldrv_pipeline {
    struct nulldrv_obj obj;
    struct nulldrv_dev *dev;
};

struct nulldrv_dynamic_vp {
    struct nulldrv_obj obj;
};

struct nulldrv_dynamic_line_width {
    struct nulldrv_obj obj;
};

struct nulldrv_dynamic_depth_bias {
    struct nulldrv_obj obj;
};

struct nulldrv_dynamic_blend {
    struct nulldrv_obj obj;
};

struct nulldrv_dynamic_depth_bounds {
    struct nulldrv_obj obj;
};

struct nulldrv_dynamic_stencil {
    struct nulldrv_obj obj;
};

struct nulldrv_cmd {
    struct nulldrv_obj obj;
};

struct nulldrv_desc_pool {
    struct nulldrv_obj obj;
    struct nulldrv_dev *dev;
};

struct nulldrv_desc_set {
    struct nulldrv_obj obj;
    struct nulldrv_desc_ooxx *ooxx;
    const struct nulldrv_desc_layout *layout;
};

struct nulldrv_framebuffer {
    struct nulldrv_obj obj;
};

struct nulldrv_render_pass {
    struct nulldrv_obj obj;
};

struct nulldrv_buf_view {
    struct nulldrv_obj obj;

    struct nulldrv_buf *buf;

    /* SURFACE_STATE */
    uint32_t cmd[8];
    uint32_t fs_cmd[8];
    uint32_t cmd_len;
};

struct nulldrv_display {
    struct nulldrv_base base;
    struct nulldrv_dev *dev;
};

struct nulldrv_swap_chain {
    struct nulldrv_base base;
    struct nulldrv_dev *dev;
};

static const VkExtensionProperties instance_extensions[NULLDRV_EXT_COUNT] =
{
	{
		.extensionName = VK_KHR_SURFACE_EXTENSION_NAME,
		.specVersion = VK_KHR_SURFACE_SPEC_VERSION,
	}
};

const VkExtensionProperties device_extensions[1] =
{
	{
		.extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		.specVersion = VK_KHR_SWAPCHAIN_SPEC_VERSION,
	}
};

static struct nulldrv_base *
nulldrv_base(void* base)
{
	return (struct nulldrv_base *) base;
}

static struct nulldrv_base *
nulldrv_base_create(struct nulldrv_dev *dev,
					size_t obj_size,
					VkDebugReportObjectTypeEXT type)
{
	struct nulldrv_base *base;

	if (!obj_size)
		obj_size = sizeof(*base);

	VK_ASSERT(obj_size >= sizeof(*base));

	base = (struct nulldrv_base*)malloc(obj_size);
	if (!base)
		return NULL;

	memset(base, 0, obj_size);

	/* Initialize pointer to loader's dispatch table with ICD_LOADER_MAGIC */
	set_loader_magic_value(base);

	if (dev == NULL) {
		/*
		 * dev is NULL when we are creating the base device object
		 * Set dev now so that debug setup happens correctly
		 */
		dev = (struct nulldrv_dev *) base;
	}


	base->get_memory_requirements = NULL;

	return base;
}

static VkResult
nulldrv_gpu_add(int devid, const char *primary_node,
				const char *render_node, struct nulldrv_gpu **gpu_ret)
{
	struct nulldrv_gpu *gpu;

	gpu = malloc(sizeof(*gpu));
	if (!gpu)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	memset(gpu, 0, sizeof(*gpu));

	/* Initialize pointer to loader's dispatch table with ICD_LOADER_MAGIC */
	set_loader_magic_value(gpu);

	*gpu_ret = gpu;

	return VK_SUCCESS;
}

static VkResult
nulldrv_queue_create(struct nulldrv_dev *dev,
					 uint32_t node_index,
					 struct nulldrv_queue **queue_ret)
{
	struct nulldrv_queue *queue;

	queue = (struct nulldrv_queue *)
		nulldrv_base_create(dev, sizeof(*queue),
							VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT);
	if (!queue)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	queue->dev = dev;

	*queue_ret = queue;

	return VK_SUCCESS;
}

static VkResult
dev_create_queues(struct nulldrv_dev *dev,
				  const VkDeviceQueueCreateInfo *queues,
				  uint32_t count)
{
	uint32_t i;

	for (i = 0; i < count; i++) {
		const VkDeviceQueueCreateInfo *q = &queues[i];
		VkResult ret = VK_SUCCESS;

		if (q->queueCount == 1 && !dev->queues[q->queueFamilyIndex]) {
			ret = nulldrv_queue_create(dev, q->queueFamilyIndex,
									   &dev->queues[q->queueFamilyIndex]);
		}

		if (ret != VK_SUCCESS)
			return ret;
	}

	return VK_SUCCESS;
}

static enum nulldrv_ext_type
nulldrv_gpu_lookup_extension(const struct nulldrv_gpu *gpu,
							 const char* name)
{
	enum nulldrv_ext_type type;

	for (type = 0; type < ARRAY_LENGTH(device_extensions); type++) {
		if (strcmp(device_extensions[type].extensionName, name) == 0)
			break;
	}

	VK_ASSERT(type < NULLDRV_EXT_COUNT || type == NULLDRV_EXT_INVALID);

	return type;
}

static VkResult
nulldrv_desc_ooxx_create(struct nulldrv_dev *dev,
						 struct nulldrv_desc_ooxx **ooxx_ret)
{
	struct nulldrv_desc_ooxx *ooxx;

	ooxx = malloc(sizeof(*ooxx));
	if (!ooxx)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	memset(ooxx, 0, sizeof(*ooxx));

	ooxx->surface_desc_size = 0;
	ooxx->sampler_desc_size = 0;

	*ooxx_ret = ooxx;

	return VK_SUCCESS;
}

static VkResult
nulldrv_dev_create(struct nulldrv_gpu *gpu,
				   const VkDeviceCreateInfo *info,
				   struct nulldrv_dev **dev_ret)
{
	struct nulldrv_dev *dev;
	uint32_t i;
	VkResult ret;

	dev = (struct nulldrv_dev *)
		nulldrv_base_create(NULL, sizeof(*dev),
							VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT);
	if (!dev)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	for (i = 0; i < info->enabledExtensionCount; i++) {
		enum nulldrv_ext_type ext =
			nulldrv_gpu_lookup_extension(gpu,
										 info->ppEnabledExtensionNames[i]);

		if (ext == NULLDRV_EXT_INVALID)
			return VK_ERROR_EXTENSION_NOT_PRESENT;

		dev->exts[ext] = true;
	}

	ret = nulldrv_desc_ooxx_create(dev, &dev->desc_ooxx);
	if (ret != VK_SUCCESS)
		return ret;

	ret = dev_create_queues(dev, info->pQueueCreateInfos,
							info->queueCreateInfoCount);
	if (ret != VK_SUCCESS)
		return ret;

	*dev_ret = dev;

	return VK_SUCCESS;
}

static struct nulldrv_gpu *nulldrv_gpu(VkPhysicalDevice gpu)
{
	return (struct nulldrv_gpu *) gpu;
}

static VkResult nulldrv_fence_create(struct nulldrv_dev *dev,
									 const VkFenceCreateInfo *info,
									 struct nulldrv_fence **fence_ret)
{
	struct nulldrv_fence *fence;

	fence = (struct nulldrv_fence *)
		nulldrv_base_create(dev, sizeof(*fence),
							VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);
	if (!fence)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*fence_ret = fence;

	return VK_SUCCESS;
}

static struct nulldrv_dev *
nulldrv_dev(VkDevice dev)
{
	return (struct nulldrv_dev *) dev;
}

static struct nulldrv_img *
nulldrv_img_from_base(struct nulldrv_base *base)
{
	return (struct nulldrv_img *) base;
}


static VkResult
img_get_memory_requirements(struct nulldrv_base *base,
							VkMemoryRequirements *requirements)
{
	struct nulldrv_img *img = nulldrv_img_from_base(base);
	VkResult ret = VK_SUCCESS;

	requirements->size = img->total_size;
	requirements->alignment = 4096;
	requirements->memoryTypeBits = ~0u;        /* can use any memory type */

	return ret;
}

static VkResult
nulldrv_img_create(struct nulldrv_dev *dev,
				   tbm_surface_h tbm_surface,
				   const VkImageCreateInfo *info,
				   bool scanout,
				   struct nulldrv_img **img_ret)
{
	struct nulldrv_img *img;

	img = (struct nulldrv_img *)
		nulldrv_base_create(dev, sizeof(*img),
							VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);
	if (!img)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	img->tbm_surface = tbm_surface;
	img->type = info->imageType;
	img->depth = info->extent.depth;
	img->mip_levels = info->mipLevels;
	img->array_size = info->arrayLayers;
	img->usage = info->usage;
	img->samples = info->samples;

	img->obj.base.get_memory_requirements = img_get_memory_requirements;

	*img_ret = img;

	return VK_SUCCESS;
}

static struct nulldrv_img *
nulldrv_img(VkImage image)
{
	return *(struct nulldrv_img **) &image;
}

static VkResult
nulldrv_mem_alloc(struct nulldrv_dev *dev,
				  const VkMemoryAllocateInfo *info,
				  struct nulldrv_mem **mem_ret)
{
	struct nulldrv_mem *mem;

	mem = (struct nulldrv_mem *)
		nulldrv_base_create(dev, sizeof(*mem),
							VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT);
	if (!mem)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	mem->bo = malloc(info->allocationSize);
	if (!mem->bo)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	mem->size = info->allocationSize;

	*mem_ret = mem;

	return VK_SUCCESS;
}

static VkResult
nulldrv_sampler_create(struct nulldrv_dev *dev,
					   const VkSamplerCreateInfo *info,
					   struct nulldrv_sampler **sampler_ret)
{
	struct nulldrv_sampler *sampler;

	sampler = (struct nulldrv_sampler *)
		nulldrv_base_create(dev, sizeof(*sampler),
							VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT);
	if (!sampler)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*sampler_ret = sampler;

	return VK_SUCCESS;
}

static VkResult
nulldrv_img_view_create(struct nulldrv_dev *dev,
						const VkImageViewCreateInfo *info,
						struct nulldrv_img_view **view_ret)
{
	struct nulldrv_img *img = nulldrv_img(info->image);
	struct nulldrv_img_view *view;

	view = (struct nulldrv_img_view *)
		nulldrv_base_create(dev, sizeof(*view),
							VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);
	if (!view)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	view->img = img;

	view->cmd_len = 8;

	*view_ret = view;

	return VK_SUCCESS;
}

static void *
nulldrv_mem_map(struct nulldrv_mem *mem, VkFlags flags)
{
	return mem->bo;
}

static struct nulldrv_mem *
nulldrv_mem(VkDeviceMemory mem)
{
	return *(struct nulldrv_mem **) &mem;
}

static struct nulldrv_buf *
nulldrv_buf_from_base(struct nulldrv_base *base)
{
	return (struct nulldrv_buf *) base;
}

static VkResult
buf_get_memory_requirements(struct nulldrv_base *base,
							VkMemoryRequirements* requirements)
{
	struct nulldrv_buf *buf = nulldrv_buf_from_base(base);

	if (requirements == NULL)
		return VK_SUCCESS;

	requirements->size = buf->size;
	requirements->alignment = 4096;
	requirements->memoryTypeBits = 1; /* nulldrv only has one memory type */

	return VK_SUCCESS;
}

static VkResult
nulldrv_buf_create(struct nulldrv_dev *dev,
				   const VkBufferCreateInfo *info,
				   struct nulldrv_buf **buf_ret)
{
	struct nulldrv_buf *buf;

	buf = (struct nulldrv_buf *)
		nulldrv_base_create(dev, sizeof(*buf),
							VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);
	if (!buf)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	buf->size = info->size;
	buf->usage = info->usage;

	buf->obj.base.get_memory_requirements = buf_get_memory_requirements;

	*buf_ret = buf;

	return VK_SUCCESS;
}

static VkResult
nulldrv_desc_layout_create(struct nulldrv_dev *dev,
						   const VkDescriptorSetLayoutCreateInfo *info,
						   struct nulldrv_desc_layout **layout_ret)
{
	struct nulldrv_desc_layout *layout;

	layout = (struct nulldrv_desc_layout *)
		nulldrv_base_create(dev, sizeof(*layout),
							VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);
	if (!layout)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*layout_ret = layout;

	return VK_SUCCESS;
}

static VkResult
nulldrv_pipeline_layout_create(struct nulldrv_dev *dev,
							   const VkPipelineLayoutCreateInfo* info,
							   struct nulldrv_pipeline_layout **ret)
{
	struct nulldrv_pipeline_layout *pipeline_layout;

	pipeline_layout = (struct nulldrv_pipeline_layout *)
		nulldrv_base_create(dev, sizeof(*pipeline_layout),
							VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);
	if (!pipeline_layout)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*ret = pipeline_layout;

	return VK_SUCCESS;
}

static struct nulldrv_desc_layout *
nulldrv_desc_layout(const VkDescriptorSetLayout layout)
{
	return *(struct nulldrv_desc_layout **) &layout;
}

static VkResult
graphics_pipeline_create(struct nulldrv_dev *dev,
						 const VkGraphicsPipelineCreateInfo *info,
						 struct nulldrv_pipeline **pipeline_ret)
{
	struct nulldrv_pipeline *pipeline;

	pipeline = (struct nulldrv_pipeline *)
		nulldrv_base_create(dev, sizeof(*pipeline),
							VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
	if (!pipeline)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*pipeline_ret = pipeline;

	return VK_SUCCESS;
}

static VkResult
nulldrv_cmd_create(struct nulldrv_dev *dev,
				   const VkCommandBufferAllocateInfo *info,
				   struct nulldrv_cmd **cmd_ret)
{
	struct nulldrv_cmd *cmd;
	uint32_t num_allocated = 0;
	uint32_t i, j;

	for (i = 0; i < info->commandBufferCount; i++) {
		cmd = (struct nulldrv_cmd *)
			nulldrv_base_create(dev, sizeof(*cmd),
								VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT);
		if (!cmd) {
			for (j = 0; j < num_allocated; j++)
				free(cmd_ret[j]);

			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		num_allocated++;
		cmd_ret[i] = cmd;
	}

	return VK_SUCCESS;
}

static VkResult
nulldrv_desc_pool_create(struct nulldrv_dev *dev,
						 const VkDescriptorPoolCreateInfo *info,
						 struct nulldrv_desc_pool **pool_ret)
{
	struct nulldrv_desc_pool *pool;

	pool = (struct nulldrv_desc_pool *)
		nulldrv_base_create(dev, sizeof(*pool),
							VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT);
	if (!pool)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	pool->dev = dev;

	*pool_ret = pool;

	return VK_SUCCESS;
}

static VkResult
nulldrv_desc_set_create(struct nulldrv_dev *dev,
						struct nulldrv_desc_pool *pool,
						const struct nulldrv_desc_layout *layout,
						struct nulldrv_desc_set **set_ret)
{
	struct nulldrv_desc_set *set;

	set = (struct nulldrv_desc_set *)
		nulldrv_base_create(dev, sizeof(*set),
							VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT);
	if (!set)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	set->ooxx = dev->desc_ooxx;
	set->layout = layout;
	*set_ret = set;

	return VK_SUCCESS;
}

static struct nulldrv_desc_pool *
nulldrv_desc_pool(VkDescriptorPool pool)
{
	return *(struct nulldrv_desc_pool **) &pool;
}

static VkResult
nulldrv_fb_create(struct nulldrv_dev *dev,
				  const VkFramebufferCreateInfo *info,
				  struct nulldrv_framebuffer **fb_ret)
{

	struct nulldrv_framebuffer *fb;

	fb = (struct nulldrv_framebuffer *)
		nulldrv_base_create(dev, sizeof(*fb),
							VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT);
	if (!fb)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*fb_ret = fb;

	return VK_SUCCESS;

}

static VkResult
nulldrv_render_pass_create(struct nulldrv_dev *dev,
						   const VkRenderPassCreateInfo *info,
						   struct nulldrv_render_pass **rp_ret)
{
	struct nulldrv_render_pass *rp;

	rp = (struct nulldrv_render_pass *)
		nulldrv_base_create(dev, sizeof(*rp),
							VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT);
	if (!rp)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*rp_ret = rp;

	return VK_SUCCESS;
}

static struct nulldrv_buf *
nulldrv_buf(VkBuffer buf)
{
	return *(struct nulldrv_buf **) &buf;
}

static VkResult
nulldrv_buf_view_create(struct nulldrv_dev *dev,
						const VkBufferViewCreateInfo *info,
						struct nulldrv_buf_view **view_ret)
{
	struct nulldrv_buf *buf = nulldrv_buf(info->buffer);
	struct nulldrv_buf_view *view;

	view = (struct nulldrv_buf_view *)
		nulldrv_base_create(dev, sizeof(*view),
							VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);
	if (!view)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	view->buf = buf;

	*view_ret = view;

	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_buffer(VkDevice device,
			  const VkBufferCreateInfo *info,
			  const VkAllocationCallbacks *allocator,
			  VkBuffer *buffer)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_buf_create(dev, info,
							  (struct nulldrv_buf **) buffer);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_buffer(VkDevice device,
			   VkBuffer buffer,
			   const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_command_pool(VkDevice device,
					const VkCommandPoolCreateInfo *info,
					const VkAllocationCallbacks *allocator,
					VkCommandPool *pool)
{
	NULLDRV_LOG_FUNC;
	*pool = (VkCommandPool)1;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_command_pool(VkDevice device,
					 VkCommandPool pool,
					 const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
reset_command_pool(VkDevice device,
				   VkCommandPool pool,
				   VkCommandPoolResetFlags flags)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
allocate_command_buffers(VkDevice device,
						 const VkCommandBufferAllocateInfo *info,
						 VkCommandBuffer *command_buffers)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_cmd_create(dev, info, (struct nulldrv_cmd **)command_buffers);
}

static VKAPI_ATTR void VKAPI_CALL
free_command_buffers(VkDevice device,
					 VkCommandPool pool,
					 uint32_t count,
					 const VkCommandBuffer *command_buffers)
{
	NULLDRV_LOG_FUNC;
	uint32_t i;

	for (i = 0; i < count; i++)
		free(command_buffers[i]);
}

static VKAPI_ATTR VkResult VKAPI_CALL
begin_command_buffer(VkCommandBuffer command_buffer,
					 const VkCommandBufferBeginInfo *info)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
end_command_buffer(VkCommandBuffer command_buffer)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
reset_command_buffer(VkCommandBuffer command_buffer,
					 VkCommandBufferResetFlags flags)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_copy_buffer(VkCommandBuffer command_buffer,
				VkBuffer src,
				VkBuffer dst,
				uint32_t region_count,
				const VkBufferCopy *regions)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_copy_image(VkCommandBuffer command_buffer,
			   VkImage src,
			   VkImageLayout src_layout,
			   VkImage dst,
			   VkImageLayout dst_layout,
			   uint32_t region_count,
			   const VkImageCopy *regions)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_blit_image(VkCommandBuffer command_buffer,
			   VkImage src,
			   VkImageLayout src_layout,
			   VkImage dst,
			   VkImageLayout dst_layout,
			   uint32_t region_count,
			   const VkImageBlit *regions,
			   VkFilter filter)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_copy_buffer_to_image(VkCommandBuffer command_buffer,
						 VkBuffer src,
						 VkImage dst,
						 VkImageLayout dst_layout,
						 uint32_t region_count,
						 const VkBufferImageCopy *regions)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_copy_image_to_buffer(VkCommandBuffer command_buffer,
						 VkImage src,
						 VkImageLayout src_layout,
						 VkBuffer dst,
						 uint32_t region_count,
						 const VkBufferImageCopy *regions)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_update_buffer(VkCommandBuffer command_buffer,
				  VkBuffer buffer,
				  VkDeviceSize offset,
				  VkDeviceSize size,
				  const uint32_t *data)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_fill_buffer(VkCommandBuffer command_buffer,
				VkBuffer buffer,
				VkDeviceSize offset,
				VkDeviceSize size,
				uint32_t data)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_clear_depth_stencil_image(VkCommandBuffer command_buffer,
							  VkImage image,
							  VkImageLayout layout,
							  const VkClearDepthStencilValue *value,
							  uint32_t range_count,
							  const VkImageSubresourceRange *ranges)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_clear_attachments(VkCommandBuffer command_buffer,
					  uint32_t attachment_count,
					  const VkClearAttachment *attachments,
					  uint32_t rect_count,
					  const VkClearRect *rects)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_clear_color_image(VkCommandBuffer command_buffer,
					  VkImage image,
					  VkImageLayout image_layout,
					  const VkClearColorValue *color,
					  uint32_t range_count,
					  const VkImageSubresourceRange *ranges)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_resolve_image(VkCommandBuffer command_buffer,
				  VkImage src,
				  VkImageLayout src_layout,
				  VkImage dst,
				  VkImageLayout dst_layout,
				  uint32_t region_count,
				  const VkImageResolve *regions)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_begin_query(VkCommandBuffer command_buffer,
				VkQueryPool pool,
				uint32_t slot,
				VkFlags flags)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_end_query(VkCommandBuffer command_buffer,
			  VkQueryPool pool,
			  uint32_t slot)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_reset_query_pool(VkCommandBuffer command_buffer,
					 VkQueryPool pool,
					 uint32_t first_query,
					 uint32_t query_count)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_event(VkCommandBuffer command_buffer,
			  VkEvent event,
			  VkPipelineStageFlags stage_mask)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_reset_event(VkCommandBuffer command_buffer,
				VkEvent event,
				VkPipelineStageFlags stage_mask)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_copy_query_pool_results(VkCommandBuffer command_buffer,
							VkQueryPool pool,
							uint32_t first_query,
							uint32_t query_count,
							VkBuffer buffer,
							VkDeviceSize offset,
							VkDeviceSize stride,
							VkFlags flags)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_write_timestamp(VkCommandBuffer command_buffer,
					VkPipelineStageFlagBits pipeline_stage,
					VkQueryPool pool,
					uint32_t slot)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_bind_pipeline(VkCommandBuffer command_buffer,
				  VkPipelineBindPoint pipeline_bind_point,
				  VkPipeline pipeline)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_viewport(VkCommandBuffer command_buffer,
				 uint32_t first,
				 uint32_t count,
				 const VkViewport *viewports)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_scissor(VkCommandBuffer command_buffer,
				uint32_t first,
				uint32_t count,
				const VkRect2D *scissors)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_line_width(VkCommandBuffer command_buffer, float line_width)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_depth_bias(VkCommandBuffer command_buffer,
				   float constant,
				   float clamp,
				   float slope)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_blend_constants(VkCommandBuffer command_buffer,
						const float blend_constants[4])
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_depth_bounds(VkCommandBuffer command_buffer,
					 float min,
					 float max)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_stencil_compare_mask(VkCommandBuffer command_buffer,
							 VkStencilFaceFlags face_mask,
							 uint32_t compare_mask)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_stencil_write_mask(VkCommandBuffer command_buffer,
						   VkStencilFaceFlags face_mask,
						   uint32_t write_mask)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_set_stencil_reference(VkCommandBuffer command_buffer,
						  VkStencilFaceFlags face_mask,
						  uint32_t reference)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_bind_descriptor_sets(VkCommandBuffer command_buffer,
						 VkPipelineBindPoint pipeline_bind_point,
						 VkPipelineLayout layout,
						 uint32_t first,
						 uint32_t count,
						 const VkDescriptorSet *sets,
						 uint32_t offset_count,
						 const uint32_t *offsets)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_bind_vertex_buffers(VkCommandBuffer command_buffer,
						uint32_t first,
						uint32_t count,
						const VkBuffer *buffers,
						const VkDeviceSize *offsets)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_bind_index_buffer(VkCommandBuffer command_buffer,
					  VkBuffer buffer,
					  VkDeviceSize offset,
					  VkIndexType index_type)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_draw(VkCommandBuffer command_buffer,
		 uint32_t vertex_count,
		 uint32_t instance_count,
		 uint32_t first_vertex,
		 uint32_t first_instance)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_draw_indexed(VkCommandBuffer command_buffer,
				 uint32_t index_count,
				 uint32_t instance_count,
				 uint32_t first_index,
				 int32_t vertex_offset,
				 uint32_t first_instance)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_draw_indirect(VkCommandBuffer command_buffer,
				  VkBuffer buffer,
				  VkDeviceSize offset,
				  uint32_t draw_count,
				  uint32_t stride)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_draw_indexed_indirect(VkCommandBuffer command_buffer,
						  VkBuffer buffer,
						  VkDeviceSize offset,
						  uint32_t draw_count,
						  uint32_t stride)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_dispatch(VkCommandBuffer command_buffer,
			 uint32_t x,
			 uint32_t y,
			 uint32_t z)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_dispatch_indirect(VkCommandBuffer command_buffer,
					  VkBuffer buffer,
					  VkDeviceSize offset)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_wait_events(VkCommandBuffer command_buffer,
				uint32_t event_count,
				const VkEvent *events,
				VkPipelineStageFlags source_stage_mask,
				VkPipelineStageFlags dst_stage_mask,
				uint32_t memory_barrier_count,
				const VkMemoryBarrier *memory_barriers,
				uint32_t buffer_memory_barrierCount,
				const VkBufferMemoryBarrier *buffer_memory_barriers,
				uint32_t image_memory_barrierCount,
				const VkImageMemoryBarrier *image_memory_barriers)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_pipeline_barrier(VkCommandBuffer command_buffer,
					 VkPipelineStageFlags src_stage_mask,
					 VkPipelineStageFlags dst_stage_mask,
					 VkDependencyFlags dependency_flags,
					 uint32_t memory_barrier_count,
					 const VkMemoryBarrier *memory_barriers,
					 uint32_t buffer_memory_barrierCount,
					 const VkBufferMemoryBarrier *buffer_memory_barriers,
					 uint32_t image_memory_barrierCount,
					 const VkImageMemoryBarrier *image_memory_barriers)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_device(VkPhysicalDevice dev,
			  const VkDeviceCreateInfo *info,
			  const VkAllocationCallbacks *allocator,
			  VkDevice *device)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_gpu *gpu = nulldrv_gpu(dev);
	return nulldrv_dev_create(gpu, info, (struct nulldrv_dev**)device);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_device(VkDevice device,
			   const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
get_device_queue(VkDevice device,
				 uint32_t queue_node_index,
				 uint32_t queue_index,
				 VkQueue *queue)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);
	*queue = (VkQueue) dev->queues[0];
}

static VKAPI_ATTR VkResult VKAPI_CALL
device_wait_idle(VkDevice device)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_event(VkDevice device,
			 const VkEventCreateInfo *info,
			 const VkAllocationCallbacks *allocator,
			 VkEvent *event)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_event(VkDevice device,
			  VkEvent event,
			  const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_event_status(VkDevice device,
				 VkEvent event)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
set_event(VkDevice device,
		  VkEvent event)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
reset_event(VkDevice device,
			VkEvent event)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_fence(VkDevice device,
			 const VkFenceCreateInfo *info,
			 const VkAllocationCallbacks *allocator,
			 VkFence *fence)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_fence_create(dev, info,
								(struct nulldrv_fence **) fence);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_fence(VkDevice device,
			  VkFence fence,
			  const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_fence_status(VkDevice device,
				 VkFence fence)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
reset_fences(VkDevice device,
			 uint32_t fence_count,
			 const VkFence *fences)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
wait_for_fences(VkDevice device,
				uint32_t fence_count,
				const VkFence *fences,
				VkBool32 wait_all,
				uint64_t timeout)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
get_physical_device_properties(VkPhysicalDevice pdev,
							   VkPhysicalDeviceProperties *props)
{
	NULLDRV_LOG_FUNC;

	props->apiVersion = VK_API_VERSION_1_0;
	props->driverVersion = 0;
	props->vendorID = 0;
	props->deviceID = 0;
	props->deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
	strncpy(props->deviceName, "nulldrv", strlen("nulldrv"));

	/* TODO: fill out limits */
	memset(&props->limits, 0, sizeof(VkPhysicalDeviceLimits));
	memset(&props->sparseProperties, 0,
		   sizeof(VkPhysicalDeviceSparseProperties));
}

static VKAPI_ATTR void VKAPI_CALL
get_physical_device_features(VkPhysicalDevice dev,
							 VkPhysicalDeviceFeatures *features)
{
	NULLDRV_LOG_FUNC;

	memset(features, 0xff, sizeof(*features));
}

static VKAPI_ATTR void VKAPI_CALL
get_physical_device_format_properties(VkPhysicalDevice pdev,
									  VkFormat format,
									  VkFormatProperties *format_info)
{
	NULLDRV_LOG_FUNC;

	format_info->linearTilingFeatures = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
		VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
	format_info->optimalTilingFeatures = format_info->linearTilingFeatures;
	format_info->bufferFeatures = 0;
}

static VKAPI_ATTR void VKAPI_CALL
get_physical_device_queue_family_properties(VkPhysicalDevice pdev,
											uint32_t *queue_family_prop_count,
											VkQueueFamilyProperties *props)
{
	if (props == NULL) {
		*queue_family_prop_count = 1;
		return;
	}
	props->queueFlags = VK_QUEUE_GRAPHICS_BIT |
		VK_QUEUE_SPARSE_BINDING_BIT;
	props->queueCount = 1;
	props->timestampValidBits = 0;
}

static VKAPI_ATTR void VKAPI_CALL
get_physical_device_memory_properties(VkPhysicalDevice pdev,
									  VkPhysicalDeviceMemoryProperties *props)
{
	/* null driver pretends to have a single memory type (and single heap) */
	props->memoryTypeCount = 1;
	props->memoryHeapCount = 1;
	props->memoryTypes[0].heapIndex = 0;
	props->memoryTypes[0].propertyFlags =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
		VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
	props->memoryHeaps[0].flags = 0; /* not physical_deviceice local */
	props->memoryHeaps[0].size = 0;  /* it's just malloc-backed memory */
}

static VKAPI_ATTR VkResult VKAPI_CALL
enumerate_device_layer_properties(VkPhysicalDevice pdev,
								  uint32_t *prop_count,
								  VkLayerProperties *props)
{
	/* TODO: Fill in with real data */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
enumerate_instance_extension_properties(const char *layer_name,
										uint32_t *prop_count,
										VkExtensionProperties *props)
{
	uint32_t copy_size;

	if (props == NULL) {
		*prop_count = NULLDRV_EXT_COUNT;
		return VK_SUCCESS;
	}

	copy_size = *prop_count < NULLDRV_EXT_COUNT ?
		*prop_count : NULLDRV_EXT_COUNT;

	memcpy(props, instance_extensions,
		   copy_size * sizeof(VkExtensionProperties));

	*prop_count = copy_size;

	if (copy_size < NULLDRV_EXT_COUNT)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
enumerate_instance_layer_properties(uint32_t *prop_count,
									VkLayerProperties *props)
{
	/* TODO: Fill in with real data */
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
enumerate_device_extension_properties(VkPhysicalDevice pdev,
									  const char *layer_name,
									  uint32_t *prop_count,
									  VkExtensionProperties *props)
{
	uint32_t copy_size;
	uint32_t extension_count = ARRAY_LENGTH(device_extensions);

	if (props == NULL) {
		*prop_count = extension_count;
		return VK_SUCCESS;
	}

	copy_size = *prop_count < extension_count ?
		*prop_count : extension_count;

	memcpy(props, device_extensions,
		   copy_size * sizeof(VkExtensionProperties));

	*prop_count = copy_size;

	if (copy_size < extension_count)
		return VK_INCOMPLETE;

	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_image(VkDevice device,
			 const VkImageCreateInfo *info,
			 const VkAllocationCallbacks *allocator,
			 VkImage *image)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_img_create(dev, NULL, info, false,
							  (struct nulldrv_img **) image);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_image(VkDevice device,
			  VkImage image,
			  const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
get_image_subresource_layout(VkDevice device,
							 VkImage image,
							 const VkImageSubresource *subresource,
							 VkSubresourceLayout *layout)
{
	NULLDRV_LOG_FUNC;

	layout->offset = 0;
	layout->size = 1;
	layout->rowPitch = 4;
	layout->depthPitch = 4;
	layout->arrayPitch = 4;
}

static VKAPI_ATTR VkResult VKAPI_CALL
allocate_memory(VkDevice device,
				const VkMemoryAllocateInfo *info,
				const VkAllocationCallbacks *allocator,
				VkDeviceMemory *memory)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_mem_alloc(dev, info,
							 (struct nulldrv_mem **) memory);
}

static VKAPI_ATTR void VKAPI_CALL
free_memory(VkDevice device,
			VkDeviceMemory memory,
			const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
map_memory(VkDevice device,
		   VkDeviceMemory memory,
		   VkDeviceSize offset,
		   VkDeviceSize size,
		   VkFlags flags,
		   void** pdata)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_mem *mem = nulldrv_mem(memory);
	void *ptr = nulldrv_mem_map(mem, flags);

	*pdata = ptr;

	return (ptr) ? VK_SUCCESS : VK_ERROR_MEMORY_MAP_FAILED;
}

static VKAPI_ATTR void VKAPI_CALL
unmap_memory(VkDevice device,
			 VkDeviceMemory memory)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
flush_mapped_memory_ranges(VkDevice device,
						   uint32_t memory_range_count,
						   const VkMappedMemoryRange *memory_ranges)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
invalidate_mapped_memory_ranges(VkDevice device,
								uint32_t memory_range_count,
								const VkMappedMemoryRange *memory_ranges)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
get_device_memory_commitment(VkDevice device,
							 VkDeviceMemory memory,
							 VkDeviceSize *committed_memory_bytes)
{
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_instance(const VkInstanceCreateInfo *info,
				const VkAllocationCallbacks *allocator,
				VkInstance *instance)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_instance *inst;

	inst = (struct nulldrv_instance *)
		nulldrv_base_create(NULL, sizeof(*inst),
							VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT);
	if (!inst)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	inst->obj.base.get_memory_requirements = NULL;

	*instance = (VkInstance) inst;

	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_instance(VkInstance instance,
				 const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
enumerate_physical_devices(VkInstance instance,
						   uint32_t *gpu_count,
						   VkPhysicalDevice *gpus)
{
	NULLDRV_LOG_FUNC;
	VkResult ret;
	struct nulldrv_gpu *gpu;

	*gpu_count = 1;
	ret = nulldrv_gpu_add(0, 0, 0, &gpu);

	if (ret == VK_SUCCESS && gpus)
		gpus[0] = (VkPhysicalDevice) gpu;

	return ret;
}

static VKAPI_ATTR void VKAPI_CALL
get_buffer_memory_requirements(VkDevice device,
							   VkBuffer buffer,
							   VkMemoryRequirements *memory_requirements)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_base *base = nulldrv_base((void*)(uintptr_t)buffer);

	base->get_memory_requirements(base, memory_requirements);
}

static VKAPI_ATTR void VKAPI_CALL
get_image_memory_requirements(VkDevice device,
							  VkImage image,
							  VkMemoryRequirements *memory_requirements)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_base *base = nulldrv_base((void*)(uintptr_t)image);

	base->get_memory_requirements(base, memory_requirements);
}

static VKAPI_ATTR VkResult VKAPI_CALL
bind_buffer_memory(VkDevice device,
				   VkBuffer buffer,
				   VkDeviceMemory memory,
				   VkDeviceSize memory_offset)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
bind_image_memory(VkDevice device,
				  VkImage image,
				  VkDeviceMemory memory,
				  VkDeviceSize memory_offset)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
get_image_sparse_memory_requirements(VkDevice device,
									 VkImage image,
									 uint32_t *count,
									 VkSparseImageMemoryRequirements *reqs)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
get_physical_device_sparse_image_format_properties(VkPhysicalDevice dev,
												   VkFormat format,
												   VkImageType type,
												   VkSampleCountFlagBits samples,
												   VkImageUsageFlags usage,
												   VkImageTiling tiling,
												   uint32_t *prop_count,
												   VkSparseImageFormatProperties *props)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
queue_bind_sparse(VkQueue queue,
				  uint32_t bind_info_count,
				  const VkBindSparseInfo *bind_info,
				  VkFence fence)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_pipeline_cache(VkDevice device,
					  const VkPipelineCacheCreateInfo *info,
					  const VkAllocationCallbacks *allocator,
					  VkPipelineCache *cache)
{

	NULLDRV_LOG_FUNC;

	struct nulldrv_dev *dev = nulldrv_dev(device);
	struct nulldrv_pipeline_cache *pipeline_cache;

	pipeline_cache = (struct nulldrv_pipeline_cache *)
		nulldrv_base_create(dev, sizeof(*pipeline_cache),
							VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT);
	if (!pipeline_cache)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*cache = (VkPipelineCache)(uintptr_t)pipeline_cache;

	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_pipeline(VkDevice device,
				 VkPipeline pipeline,
				 const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_pipeline_cache(VkDevice device,
					   VkPipelineCache pipeline_cache,
					   const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_pipeline_cache_data(VkDevice device,
						VkPipelineCache pipeline_cache,
						size_t *data_size,
						void *data)
{
	NULLDRV_LOG_FUNC;
	return VK_ERROR_INITIALIZATION_FAILED;
}

static VKAPI_ATTR VkResult VKAPI_CALL
merge_pipeline_caches(VkDevice device,
					  VkPipelineCache dst_cache,
					  uint32_t src_cache_count,
					  const VkPipelineCache *src_caches)
{
	NULLDRV_LOG_FUNC;
	return VK_ERROR_INITIALIZATION_FAILED;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_graphics_pipelines(VkDevice device,
						  VkPipelineCache pipeline_cache,
						  uint32_t create_info_count,
						  const VkGraphicsPipelineCreateInfo *info,
						  const VkAllocationCallbacks *allocator,
						  VkPipeline *pipeline)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return graphics_pipeline_create(dev, info,
									(struct nulldrv_pipeline **) pipeline);
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_compute_pipelines(VkDevice device,
						 VkPipelineCache pipeline_cache,
						 uint32_t create_info_count,
						 const VkComputePipelineCreateInfo *info,
						 const VkAllocationCallbacks *allocator,
						 VkPipeline *pipeline)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_query_pool(VkDevice device,
				  const VkQueryPoolCreateInfo *info,
				  const VkAllocationCallbacks *allocator,
				  VkQueryPool *pool)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_query_pool(VkDevice device,
				   VkQueryPool query_poool,
				   const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_query_pool_results(VkDevice device,
					   VkQueryPool query_pool,
					   uint32_t first_query,
					   uint32_t query_count,
					   size_t data_size,
					   void *data,
					   size_t stride,
					   VkQueryResultFlags flags)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
queue_wait_idle(VkQueue queue)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
queue_submit(VkQueue queue,
			 uint32_t submit_count,
			 const VkSubmitInfo *submits,
			 VkFence fence)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_semaphore(VkDevice device,
				 const VkSemaphoreCreateInfo *info,
				 const VkAllocationCallbacks *allocator,
				 VkSemaphore *semaphore)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_semaphore(VkDevice device,
				  VkSemaphore semaphore,
				  const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_sampler(VkDevice device,
			   const VkSamplerCreateInfo *info,
			   const VkAllocationCallbacks *allocator,
			   VkSampler *sampler)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_sampler_create(dev, info,
								  (struct nulldrv_sampler **) sampler);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_sampler(VkDevice device,
				VkSampler sampler,
				const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_shader_module(VkDevice device,
					 const VkShaderModuleCreateInfo *info,
					 const VkAllocationCallbacks *allocator,
					 VkShaderModule *module)
{
	NULLDRV_LOG_FUNC;

	struct nulldrv_dev *dev = nulldrv_dev(device);
	struct nulldrv_shader_module *shader_module;

	shader_module = (struct nulldrv_shader_module *)
		nulldrv_base_create(dev, sizeof(*shader_module),
							VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT);
	if (!shader_module)
		return VK_ERROR_OUT_OF_HOST_MEMORY;

	*module = (VkShaderModule)(uintptr_t)shader_module;

	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
destroy_shader_module(VkDevice device,
					  VkShaderModule shader_module,
					  const VkAllocationCallbacks *allocator)
{
	/* TODO: Fill in with real data */
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_buffer_view(VkDevice device,
				   const VkBufferViewCreateInfo *info,
				   const VkAllocationCallbacks *allocator,
				   VkBufferView *view)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_buf_view_create(dev, info,
								   (struct nulldrv_buf_view **) view);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_buffer_view(VkDevice device,
					VkBufferView buffer_view,
					const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_image_view(VkDevice device,
				  const VkImageViewCreateInfo *info,
				  const VkAllocationCallbacks *allocator,
				  VkImageView *view)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_img_view_create(dev, info,
								   (struct nulldrv_img_view **) view);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_image_view(VkDevice device,
				   VkImageView image_view,
				   const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_descriptor_set_layout(VkDevice device,
							 const VkDescriptorSetLayoutCreateInfo *info,
							 const VkAllocationCallbacks *allocator,
							 VkDescriptorSetLayout *layout)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_desc_layout_create(dev, info,
									  (struct nulldrv_desc_layout **)layout);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_descriptor_set_layout(VkDevice device,
							  VkDescriptorSetLayout layout,
							  const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_pipeline_layout(VkDevice device,
					   const VkPipelineLayoutCreateInfo *info,
					   const VkAllocationCallbacks *allocator,
					   VkPipelineLayout *layout)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_pipeline_layout_create(dev, info,
										  (struct nulldrv_pipeline_layout **)
										  layout);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_pipeline_layout(VkDevice device,
						VkPipelineLayout layout,
						const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_descriptor_pool(VkDevice device,
					   const VkDescriptorPoolCreateInfo *info,
					   const VkAllocationCallbacks *allocator,
					   VkDescriptorPool *pool)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_desc_pool_create(dev, info,
									(struct nulldrv_desc_pool **)pool);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_descriptor_pool(VkDevice device,
						VkDescriptorPool descriptor_pool,
						const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
reset_descriptor_pool(VkDevice device,
					  VkDescriptorPool descriptor_pool,
					  VkDescriptorPoolResetFlags flags)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL
allocate_descriptor_sets(VkDevice device,
						 const VkDescriptorSetAllocateInfo *info,
						 VkDescriptorSet *sets)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_desc_pool *pool =
		nulldrv_desc_pool(info->descriptorPool);
	struct nulldrv_dev *dev = pool->dev;
	VkResult ret = VK_SUCCESS;
	uint32_t i;

	for (i = 0; i < info->descriptorSetCount; i++) {
		const struct nulldrv_desc_layout *layout =
			nulldrv_desc_layout(info->pSetLayouts[i]);

		ret = nulldrv_desc_set_create(dev, pool, layout,
									  (struct nulldrv_desc_set **)&sets[i]);
		if (ret != VK_SUCCESS)
			break;
	}

	return ret;
}

static VKAPI_ATTR VkResult VKAPI_CALL
free_descriptor_sets(VkDevice device,
					 VkDescriptorPool descriptor_pool,
					 uint32_t descriptor_set_count,
					 const VkDescriptorSet *sets)
{
	NULLDRV_LOG_FUNC;
	return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL
update_descriptor_sets(VkDevice device,
					   uint32_t write_count,
					   const VkWriteDescriptorSet *writes,
					   uint32_t copy_count,
					   const VkCopyDescriptorSet *copies)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_framebuffer(VkDevice device,
				   const VkFramebufferCreateInfo *info,
				   const VkAllocationCallbacks *allocator,
				   VkFramebuffer* fb)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_fb_create(dev, info, (struct nulldrv_framebuffer **)fb);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_framebuffer(VkDevice device,
					VkFramebuffer framebuffer,
					const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
create_render_pass(VkDevice device,
				   const VkRenderPassCreateInfo *info,
				   const VkAllocationCallbacks *allocator,
				   VkRenderPass *rp)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);

	return nulldrv_render_pass_create(dev, info,
									  (struct nulldrv_render_pass **)rp);
}

static VKAPI_ATTR void VKAPI_CALL
destroy_render_pass(VkDevice device,
					VkRenderPass render_pass,
					const VkAllocationCallbacks *allocator)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_push_constants(VkCommandBuffer command_buffer,
				   VkPipelineLayout layout,
				   VkShaderStageFlags stage_flags,
				   uint32_t offset,
				   uint32_t size,
				   const void *values)
{
	/* TODO: Implement */
}

static VKAPI_ATTR void VKAPI_CALL
get_render_area_granularity(VkDevice device,
							VkRenderPass render_pass,
							VkExtent2D *granularity)
{
	granularity->height = 1;
	granularity->width = 1;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_begin_render_pass(VkCommandBuffer command_buffer,
					  const VkRenderPassBeginInfo *render_pass_begin,
					  VkSubpassContents contents)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_next_subpass(VkCommandBuffer command_buffer,
				 VkSubpassContents contents)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_end_render_pass(VkCommandBuffer command_buffer)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR void VKAPI_CALL
cmd_execute_commands(VkCommandBuffer command_buffer,
					 uint32_t count,
					 const VkCommandBuffer *buffers)
{
	NULLDRV_LOG_FUNC;
}

static VKAPI_ATTR VkResult VKAPI_CALL
get_physical_device_image_format_properties(VkPhysicalDevice dev,
											VkFormat format,
											VkImageType type,
											VkImageTiling tiling,
											VkImageUsageFlags usage,
											VkImageCreateFlags flags,
											VkImageFormatProperties *props)
{
	props->maxExtent.width = 1024;
	props->maxExtent.height = 1024;
	props->maxExtent.depth = 1024;
	props->maxMipLevels = 10;
	props->maxArrayLayers = 1024;
	props->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
	props->maxResourceSize = 1024*1024*1024;

	return VK_SUCCESS;
}

struct nulldrv_entry
{
	const char 	*name;
	void		*func;
};

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
get_device_proc_addr(VkDevice device, const char *name);

static const struct nulldrv_entry device_funcs[] =
{
	{ "vkGetDeviceProcAddr", get_device_proc_addr },
	{ "vkEnumerateDeviceExtensionProperties", enumerate_device_extension_properties },
	{ "vkEnumerateDeviceLayerProperties", enumerate_device_layer_properties },

	{ "vkDestroyDevice", destroy_device },
	{ "vkGetDeviceQueue", get_device_queue },
	{ "vkQueueSubmit", queue_submit },
	{ "vkQueueWaitIdle", queue_wait_idle },
	{ "vkDeviceWaitIdle", device_wait_idle },
	{ "vkAllocateMemory", allocate_memory },
	{ "vkFreeMemory", free_memory },
	{ "vkMapMemory", map_memory },
	{ "vkUnmapMemory", unmap_memory },
	{ "vkFlushMappedMemoryRanges", flush_mapped_memory_ranges },
	{ "vkInvalidateMappedMemoryRanges", invalidate_mapped_memory_ranges },
	{ "vkGetDeviceMemoryCommitment", get_device_memory_commitment },
	{ "vkBindBufferMemory", bind_buffer_memory },
	{ "vkBindImageMemory", bind_image_memory },
	{ "vkGetBufferMemoryRequirements", get_buffer_memory_requirements },
	{ "vkGetImageMemoryRequirements", get_image_memory_requirements },
	{ "vkGetImageSparseMemoryRequirements", get_image_sparse_memory_requirements },
	{ "vkQueueBindSparse", queue_bind_sparse },
	{ "vkCreateFence", create_fence },
	{ "vkDestroyFence", destroy_fence },
	{ "vkResetFences", reset_fences },
	{ "vkGetFenceStatus", get_fence_status },
	{ "vkWaitForFences", wait_for_fences },
	{ "vkCreateSemaphore", create_semaphore },
	{ "vkDestroySemaphore", destroy_semaphore },
	{ "vkCreateEvent", create_event },
	{ "vkDestroyEvent", destroy_event },
	{ "vkGetEventStatus", get_event_status },
	{ "vkSetEvent", set_event },
	{ "vkResetEvent", reset_event },
	{ "vkCreateQueryPool", create_query_pool },
	{ "vkDestroyQueryPool", destroy_query_pool },
	{ "vkGetQueryPoolResults", get_query_pool_results },
	{ "vkCreateBuffer", create_buffer },
	{ "vkDestroyBuffer", destroy_buffer },
	{ "vkCreateBufferView", create_buffer_view },
	{ "vkDestroyBufferView", destroy_buffer_view },
	{ "vkCreateImage", create_image },
	{ "vkDestroyImage", destroy_image },
	{ "vkGetImageSubresourceLayout", get_image_subresource_layout },
	{ "vkCreateImageView", create_image_view },
	{ "vkDestroyImageView", destroy_image_view },
	{ "vkCreateShaderModule", create_shader_module },
	{ "vkDestroyShaderModule", destroy_shader_module },
	{ "vkCreatePipelineCache", create_pipeline_cache },
	{ "vkDestroyPipelineCache", destroy_pipeline_cache },
	{ "vkGetPipelineCacheData", get_pipeline_cache_data },
	{ "vkMergePipelineCaches", merge_pipeline_caches },
	{ "vkCreateGraphicsPipelines", create_graphics_pipelines },
	{ "vkCreateComputePipelines", create_compute_pipelines },
	{ "vkDestroyPipeline", destroy_pipeline },
	{ "vkCreatePipelineLayout", create_pipeline_layout },
	{ "vkDestroyPipelineLayout", destroy_pipeline_layout },
	{ "vkCreateSampler", create_sampler },
	{ "vkDestroySampler", destroy_sampler },
	{ "vkCreateDescriptorSetLayout", create_descriptor_set_layout },
	{ "vkDestroyDescriptorSetLayout", destroy_descriptor_set_layout },
	{ "vkCreateDescriptorPool", create_descriptor_pool },
	{ "vkDestroyDescriptorPool", destroy_descriptor_pool },
	{ "vkResetDescriptorPool", reset_descriptor_pool },
	{ "vkAllocateDescriptorSets", allocate_descriptor_sets },
	{ "vkFreeDescriptorSets", free_descriptor_sets },
	{ "vkUpdateDescriptorSets", update_descriptor_sets },
	{ "vkCreateFramebuffer", create_framebuffer },
	{ "vkDestroyFramebuffer", destroy_framebuffer },
	{ "vkCreateRenderPass", create_render_pass },
	{ "vkDestroyRenderPass", destroy_render_pass },
	{ "vkGetRenderAreaGranularity", get_render_area_granularity },
	{ "vkCreateCommandPool", create_command_pool },
	{ "vkDestroyCommandPool", destroy_command_pool },
	{ "vkResetCommandPool", reset_command_pool },
	{ "vkAllocateCommandBuffers", allocate_command_buffers },
	{ "vkFreeCommandBuffers", free_command_buffers },
	{ "vkBeginCommandBuffer", begin_command_buffer },
	{ "vkEndCommandBuffer", end_command_buffer },
	{ "vkResetCommandBuffer", reset_command_buffer },
	{ "vkCmdBindPipeline", cmd_bind_pipeline },
	{ "vkCmdSetViewport", cmd_set_viewport },
	{ "vkCmdSetScissor", cmd_set_scissor },
	{ "vkCmdSetLineWidth", cmd_set_line_width },
	{ "vkCmdSetDepthBias", cmd_set_depth_bias },
	{ "vkCmdSetBlendConstants", cmd_set_blend_constants },
	{ "vkCmdSetDepthBounds", cmd_set_depth_bounds },
	{ "vkCmdSetStencilCompareMask", cmd_set_stencil_compare_mask },
	{ "vkCmdSetStencilWriteMask", cmd_set_stencil_write_mask },
	{ "vkCmdSetStencilReference", cmd_set_stencil_reference },
	{ "vkCmdBindDescriptorSets", cmd_bind_descriptor_sets },
	{ "vkCmdBindIndexBuffer", cmd_bind_index_buffer },
	{ "vkCmdBindVertexBuffers", cmd_bind_vertex_buffers },
	{ "vkCmdDraw", cmd_draw },
	{ "vkCmdDrawIndexed", cmd_draw_indexed },
	{ "vkCmdDrawIndirect", cmd_draw_indirect },
	{ "vkCmdDrawIndexedIndirect", cmd_draw_indexed_indirect },
	{ "vkCmdDispatch", cmd_dispatch },
	{ "vkCmdDispatchIndirect", cmd_dispatch_indirect },
	{ "vkCmdCopyBuffer", cmd_copy_buffer },
	{ "vkCmdCopyImage", cmd_copy_image },
	{ "vkCmdBlitImage", cmd_blit_image },
	{ "vkCmdCopyBufferToImage", cmd_copy_buffer_to_image },
	{ "vkCmdCopyImageToBuffer", cmd_copy_image_to_buffer },
	{ "vkCmdUpdateBuffer", cmd_update_buffer },
	{ "vkCmdFillBuffer", cmd_fill_buffer },
	{ "vkCmdClearColorImage", cmd_clear_color_image },
	{ "vkCmdClearDepthStencilImage", cmd_clear_depth_stencil_image },
	{ "vkCmdClearAttachments", cmd_clear_attachments },
	{ "vkCmdResolveImage", cmd_resolve_image },
	{ "vkCmdSetEvent", cmd_set_event },
	{ "vkCmdResetEvent", cmd_reset_event },
	{ "vkCmdWaitEvents", cmd_wait_events },
	{ "vkCmdPipelineBarrier", cmd_pipeline_barrier },
	{ "vkCmdBeginQuery", cmd_begin_query },
	{ "vkCmdEndQuery", cmd_end_query },
	{ "vkCmdResetQueryPool", cmd_reset_query_pool },
	{ "vkCmdWriteTimestamp", cmd_write_timestamp },
	{ "vkCmdCopyQueryPoolResults", cmd_copy_query_pool_results },
	{ "vkCmdPushConstants", cmd_push_constants },
	{ "vkCmdBeginRenderPass", cmd_begin_render_pass },
	{ "vkCmdNextSubpass", cmd_next_subpass },
	{ "vkCmdEndRenderPass", cmd_end_render_pass },
	{ "vkCmdExecuteCommands", cmd_execute_commands },
};

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
get_device_proc_addr(VkDevice device, const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_LENGTH(device_funcs); ++i) {
		if (strcmp(name, device_funcs[i].name) == 0)
			return device_funcs[i].func;
	}

	return NULL;
}

static const struct nulldrv_entry global_funcs[] =
{
	{ "vkEnumerateInstanceLayerProperties", enumerate_instance_layer_properties },
	{ "vkEnumerateInstanceExtensionProperties", enumerate_instance_extension_properties },
	{ "vkCreateInstance", create_instance },
};

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
get_instance_proc_addr(VkInstance instance, const char *name);

static const struct nulldrv_entry instance_funcs[] =
{
	{ "vkGetInstanceProcAddr", get_instance_proc_addr },
	{ "vkDestroyInstance", destroy_instance },
	{ "vkEnumeratePhysicalDevices", enumerate_physical_devices },

	{ "vkGetPhysicalDeviceFeatures", get_physical_device_features },
	{ "vkGetPhysicalDeviceFormatProperties", get_physical_device_format_properties },
	{ "vkGetPhysicalDeviceImageFormatProperties", get_physical_device_image_format_properties },
	{ "vkGetPhysicalDeviceProperties", get_physical_device_properties },
	{ "vkGetPhysicalDeviceQueueFamilyProperties", get_physical_device_queue_family_properties },
	{ "vkGetPhysicalDeviceMemoryProperties", get_physical_device_memory_properties },
	{ "vkGetPhysicalDeviceSparseImageFormatProperties",
		get_physical_device_sparse_image_format_properties },
	{ "vkCreateDevice", create_device },
};

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
get_instance_proc_addr(VkInstance instance, const char *name)
{
	unsigned int i;

	if (instance == NULL)
	{
		for (i = 0; i < ARRAY_LENGTH(global_funcs); ++i) {
			if (strcmp(name, global_funcs[i].name) == 0)
				return global_funcs[i].func;
		}
	}
	else
	{
		for (i = 0; i < ARRAY_LENGTH(instance_funcs); ++i) {
			if (strcmp(name, instance_funcs[i].name) == 0)
				return instance_funcs[i].func;
		}

		for (i = 0; i < ARRAY_LENGTH(device_funcs); ++i) {
			if (strcmp(name, device_funcs[i].name) == 0)
				return device_funcs[i].func;
		}
	}

	return NULL;
}

VK_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_LENGTH(global_funcs); ++i) {
		if (strcmp(name, global_funcs[i].name) == 0)
			return global_funcs[i].func;
	}

	for (i = 0; i < ARRAY_LENGTH(instance_funcs); ++i) {
		if (strcmp(name, instance_funcs[i].name) == 0)
			return instance_funcs[i].func;
	}

	for (i = 0; i < ARRAY_LENGTH(device_funcs); ++i) {
		if (strcmp(name, device_funcs[i].name) == 0)
			return device_funcs[i].func;
	}

	return NULL;
}

VK_EXPORT VkImage
vk_create_presentable_image(VkDevice device, const VkImageCreateInfo *info, tbm_surface_h surface)
{
	NULLDRV_LOG_FUNC;
	struct nulldrv_dev *dev = nulldrv_dev(device);
	struct nulldrv_img *img;

	if (nulldrv_img_create(dev, surface, info, false, &img) == VK_SUCCESS)
		return (VkImage)(uintptr_t)img;

	return (VkImage)(uintptr_t)NULL;
}
