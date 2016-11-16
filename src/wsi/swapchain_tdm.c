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
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

typedef struct vk_swapchain_tdm vk_swapchain_tdm_t;

struct vk_swapchain_tdm {
	tdm_display				*tdm_display;
	tdm_output				*tdm_output;
	tdm_layer				*tdm_layer;
	const tdm_output_mode	*tdm_mode;
	tdm_output_dpms			 tdm_dpms;

	tbm_surface_queue_h		 tbm_queue;

	VkPresentModeKHR		 present_mode;
	tbm_surface_h			*buffers;
	tbm_surface_h			 front_buffer;
	pthread_mutex_t			 front_mutex;
	pthread_mutex_t			 free_queue_mutex;
	pthread_cond_t			 free_queue_cond;
};

static int swapchain_tdm_timeline_key;
static int swapchain_tdm_timestamp_key;

static void
swapchain_tdm_increase_timestamp(tbm_surface_h tbm_surface)
{
	tbm_fd		 timeline;
	if (tbm_surface_internal_get_user_data(tbm_surface,
										   (unsigned long)&swapchain_tdm_timeline_key,
										   (void **)(&timeline))) {
		if (timeline != -1) {
			if (tbm_sync_timeline_inc(timeline, 1) == 0) {
				char buf[1024];
				strerror_r(errno, buf, sizeof(buf));
				VK_ERROR("Failed to increase TBM sync timeline: %d(%s)", errno, buf);
			}
		}
	}
}

static void
swapchain_tdm_output_commit_cb(tdm_output *output, unsigned int sequence,
							   unsigned int tv_sec, unsigned int tv_usec,
							   void *user_data)
{
	vk_swapchain_t			*chain = user_data;
	vk_swapchain_tdm_t		*swapchain_tdm = chain->backend_data;

	if (pthread_mutex_lock(&swapchain_tdm->front_mutex))
		VK_ERROR("pthread_mutex_lock front buffer failed\n");

	if (swapchain_tdm->front_buffer) {
		swapchain_tdm_increase_timestamp(swapchain_tdm->front_buffer);
		swapchain_tdm->front_buffer = NULL;
	}
	pthread_mutex_unlock(&swapchain_tdm->front_mutex);
}

static VkResult
swapchain_tdm_queue_present_image(VkQueue					 queue,
								  vk_swapchain_t			*chain,
								  tbm_surface_h				 tbm_surface,
								  int						 sync_fd)
{
	tbm_surface_queue_error_e	 tsq_err;
	tdm_error					 tdm_err;
	vk_swapchain_tdm_t			*swapchain_tdm = chain->backend_data;

	if (sync_fd != -1) {
		if (tbm_sync_fence_wait(sync_fd, -1) != 1) {
			char buf[1024];
			strerror_r(errno, buf, sizeof(buf));
			VK_ERROR("Failed to wait sync. | error: %d(%s)", errno, buf);
		}
		close(sync_fd);
	}

	tsq_err = tbm_surface_queue_enqueue(swapchain_tdm->tbm_queue, tbm_surface);
	VK_CHECK(tsq_err == TBM_SURFACE_QUEUE_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tbm_surface_queue_enqueue failed.\n");

	tsq_err = tbm_surface_queue_acquire(swapchain_tdm->tbm_queue, &tbm_surface);
	VK_CHECK(tsq_err == TBM_SURFACE_QUEUE_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tbm_surface_queue_acquire failed.\n");

	tdm_err = tdm_layer_set_buffer(swapchain_tdm->tdm_layer, tbm_surface);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tdm_layer_set_buffer failed.\n");

	tdm_err = tdm_output_commit(swapchain_tdm->tdm_output, 0,
								swapchain_tdm_output_commit_cb, chain);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tdm_output_commit failed.\n");

	tdm_err = tdm_display_handle_events(swapchain_tdm->tdm_display);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tdm_display_handle_events failed.\n");

	if (pthread_mutex_lock(&swapchain_tdm->free_queue_mutex))
		VK_ERROR("pthread_mutex_lock free queue failed\n");

	tsq_err = tbm_surface_queue_release(swapchain_tdm->tbm_queue, tbm_surface);
	VK_CHECK(tsq_err == TBM_SURFACE_QUEUE_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tbm_surface_queue_release failed.\n");

	pthread_mutex_unlock(&swapchain_tdm->free_queue_mutex);
	pthread_cond_signal(&swapchain_tdm->free_queue_cond);

	if (pthread_mutex_lock(&swapchain_tdm->front_mutex))
		VK_ERROR("pthread_mutex_lock front buffer failed\n");
	if (swapchain_tdm->front_buffer)
		swapchain_tdm_increase_timestamp(swapchain_tdm->front_buffer);
	swapchain_tdm->front_buffer = tbm_surface;
	pthread_mutex_unlock(&swapchain_tdm->front_mutex);

	return VK_SUCCESS;
}

static void
swapchain_tdm_timeline_destroy_cb(void *user_data)
{
	if (((tbm_fd)user_data) != -1)
		close((tbm_fd)user_data);
}

static tbm_fd
swapchain_tdm_get_sync_fence(tbm_surface_h tbm_surface)
{
	tbm_fd		 timeline;
	uint32_t	 timestamp;
	tbm_fd		 fence = -1;

	if (tbm_surface_internal_get_user_data(tbm_surface,
										   (unsigned long)&swapchain_tdm_timeline_key,
										   (void **)(&timeline))) {
		char name[32];

		tbm_surface_internal_get_user_data(tbm_surface,
										   (unsigned long)&swapchain_tdm_timestamp_key,
										   (void **)(&timestamp));
		timestamp++;
		tbm_surface_internal_set_user_data(tbm_surface,
										   (unsigned long)&swapchain_tdm_timestamp_key,
										   (void *)timestamp);

		snprintf(name, 32, "%d",
				 tbm_bo_export(tbm_surface_internal_get_bo(tbm_surface, 0)));
		fence = tbm_sync_fence_create(timeline,
									  name,
									  timestamp);
		if (fence == -1) {
			char buf[1024];
			strerror_r(errno, buf, sizeof(buf));
			VK_ERROR("Failed to create TBM sync fence: %d(%s)", errno, buf);
		}
	} else {
		/* make timeline and timestamp */
		timeline = tbm_sync_timeline_create();
		tbm_surface_internal_add_user_data(tbm_surface,
										   (unsigned long)&swapchain_tdm_timeline_key,
										   swapchain_tdm_timeline_destroy_cb);
		tbm_surface_internal_set_user_data(tbm_surface,
										   (unsigned long)&swapchain_tdm_timeline_key,
										   (void *)timeline);
		if (timeline == -1) {
			char buf[1024];
			strerror_r(errno, buf, sizeof(buf));
			VK_ERROR("Failed to create TBM sync timeline: %d(%s)", errno, buf);
		} else {
			tbm_surface_internal_set_user_data(tbm_surface,
											   (unsigned long)&swapchain_tdm_timestamp_key,
											   (void *)0);
		}
	}
	return fence;
};

static VkResult
swapchain_tdm_acquire_next_image(VkDevice			 device,
								 vk_swapchain_t		*chain,
								 uint64_t			 timeout,
								 tbm_surface_h		*tbm_surface,
								 int				*sync)
{
	tbm_surface_queue_error_e	 tsq_err;
	vk_swapchain_tdm_t			*swapchain_tdm = chain->backend_data;
	struct timespec				 abs_time;

	if (timeout != UINT64_MAX) {
		clock_gettime(CLOCK_REALTIME, &abs_time);
		abs_time.tv_sec += (timeout / 1000000000L);
		abs_time.tv_nsec += (timeout % 1000000000L);
		if (abs_time.tv_nsec >= 1000000000L) {
			abs_time.tv_sec += (abs_time.tv_nsec / 1000000000L);
			abs_time.tv_nsec = (abs_time.tv_nsec % 1000000000L);
		}
	}

	if (pthread_mutex_lock(&swapchain_tdm->free_queue_mutex))
		VK_ERROR("pthread_mutex_lock free queue failed\n");

	while (tbm_surface_queue_can_dequeue(swapchain_tdm->tbm_queue, 0) == 0) {
		if (timeout != UINT64_MAX) {
			int ret;
			ret = pthread_cond_timedwait(&swapchain_tdm->free_queue_cond,
										 &swapchain_tdm->free_queue_mutex,
										 &abs_time);
			if (ret == ETIMEDOUT) {
				/* timeout */
				pthread_mutex_unlock(&swapchain_tdm->free_queue_mutex);
				return VK_TIMEOUT;
			}
		} else {
			pthread_cond_wait(&swapchain_tdm->free_queue_cond,
							  &swapchain_tdm->free_queue_mutex);
		}
	}

	tsq_err = tbm_surface_queue_dequeue(swapchain_tdm->tbm_queue, tbm_surface);
	VK_CHECK(tsq_err == TBM_SURFACE_QUEUE_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tbm_surface_queue_dequeue failed.\n");
	pthread_mutex_unlock(&swapchain_tdm->free_queue_mutex);

	if (sync)
		*sync = swapchain_tdm_get_sync_fence(*tbm_surface);

	return VK_SUCCESS;
}

static void
swapchain_tdm_deinit(VkDevice		 device,
					 vk_swapchain_t *chain)
{
	vk_swapchain_tdm_t			*swapchain_tdm = chain->backend_data;

	if (swapchain_tdm) {
		tdm_output_set_dpms(swapchain_tdm->tdm_output, swapchain_tdm->tdm_dpms);

		pthread_cond_destroy(&swapchain_tdm->free_queue_cond);
		pthread_mutex_destroy(&swapchain_tdm->free_queue_mutex);
		pthread_mutex_destroy(&swapchain_tdm->front_mutex);

		if (swapchain_tdm->tbm_queue)
			tbm_surface_queue_destroy(swapchain_tdm->tbm_queue);

		if (swapchain_tdm->buffers)
			vk_free(&chain->allocator, swapchain_tdm->buffers);
		vk_free(&chain->allocator, swapchain_tdm);
	}
}

static VkResult
swapchain_tdm_get_buffers(VkDevice			 device,
						  vk_swapchain_t	*chain,
						  tbm_surface_h	   **buffers,
						  uint32_t			*buffer_count)
{
	uint32_t					 i;
	tbm_surface_queue_error_e	 tsq_err;
	tdm_error					 tdm_err;
	tbm_surface_info_s			 surf_info;
	tdm_info_layer				 tdm_info;
	vk_swapchain_tdm_t			*swapchain_tdm = chain->backend_data;

	*buffer_count = tbm_surface_queue_get_size(swapchain_tdm->tbm_queue);
	swapchain_tdm->buffers = vk_alloc(&chain->allocator,
											   sizeof(tbm_surface_h) * *buffer_count,
											   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(swapchain_tdm->buffers, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	for (i = 0; i < *buffer_count; i++) {
		tsq_err = tbm_surface_queue_dequeue(swapchain_tdm->tbm_queue,
											&swapchain_tdm->buffers[i]);
		VK_CHECK(tsq_err == TBM_SURFACE_QUEUE_ERROR_NONE,
				 return VK_ERROR_SURFACE_LOST_KHR,
				 "tbm_surface_queue_dequeue failed.\n");
	}

	for (i = 0; i < *buffer_count; i++) {
		tsq_err = tbm_surface_queue_release(swapchain_tdm->tbm_queue,
											swapchain_tdm->buffers[i]);
		VK_CHECK(tsq_err == TBM_SURFACE_QUEUE_ERROR_NONE,
				 return VK_ERROR_SURFACE_LOST_KHR,
				 "tbm_surface_queue_enqueue failed.\n");
	}

	*buffers = swapchain_tdm->buffers;

	tdm_err = tdm_output_get_dpms(swapchain_tdm->tdm_output, &swapchain_tdm->tdm_dpms);
	tdm_err = tdm_output_set_dpms(swapchain_tdm->tdm_output, TDM_OUTPUT_DPMS_ON);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tdm_output_set_dpms failed.\n");

	tdm_err = tdm_output_set_mode(swapchain_tdm->tdm_output,
								  swapchain_tdm->tdm_mode);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tdm_output_set_mode failed.\n");

	tbm_surface_get_info((*buffers)[0], &surf_info);

	/* from enlightenment */
	tdm_info.src_config.size.h = surf_info.planes[0].stride;
	tdm_info.src_config.size.v = surf_info.height;

	tdm_info.src_config.pos.x = 0;
	tdm_info.src_config.pos.y = 0;
	tdm_info.src_config.pos.w = surf_info.width;
	tdm_info.src_config.pos.h = surf_info.height;

	tdm_info.src_config.format = surf_info.format;

	tdm_info.dst_pos.x = 0;
	tdm_info.dst_pos.y = 0;
	tdm_info.dst_pos.w = surf_info.width;
	tdm_info.dst_pos.h = surf_info.height;

	tdm_info.transform = TDM_TRANSFORM_NORMAL;

	tdm_err = tdm_layer_set_info(swapchain_tdm->tdm_layer, &tdm_info);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tdm_layer_set_info failed.\n");

/*	tdm_err = tdm_layer_set_buffer_queue(swapchain_tdm->tdm_layer,
										 swapchain_tdm->tbm_queue);
	VK_CHECK(tdm_err == TDM_ERROR_NONE, return VK_ERROR_SURFACE_LOST_KHR,
			 "tdm_layer_set_buffer_queue failed.\n");*/

	return VK_SUCCESS;
}

VkResult
swapchain_tdm_init(VkDevice							 device,
				   const VkSwapchainCreateInfoKHR	*info,
				   vk_swapchain_t					*chain,
				   tbm_format						 format)
{
	VkIcdSurfaceDisplay	*surface = (VkIcdSurfaceDisplay *)(uintptr_t)info->surface;
	vk_display_mode_t	*disp_mode = (vk_display_mode_t *)(uintptr_t)surface->displayMode;
	vk_display_t		*disp = disp_mode->display;
	vk_swapchain_tdm_t	*swapchain_tdm;

	swapchain_tdm = vk_alloc(&chain->allocator, sizeof(vk_swapchain_tdm_t),
							 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
	VK_CHECK(swapchain_tdm, return VK_ERROR_OUT_OF_HOST_MEMORY, "vk_alloc() failed.\n");

	memset(swapchain_tdm, 0x00, sizeof(*swapchain_tdm));
	chain->backend_data = swapchain_tdm;

	swapchain_tdm->tdm_display = disp->pdev->tdm_display;
	swapchain_tdm->tdm_output = disp->tdm_output;
	swapchain_tdm->tdm_layer = disp->pdev->planes[surface->planeIndex].tdm_layer;

	swapchain_tdm->tbm_queue =
	/*	tbm_surface_queue_sequence_create(info->minImageCount,
										  info->imageExtent.width,
										  info->imageExtent.height,
										  format, TBM_BO_SCANOUT);*/
		tbm_surface_queue_create(info->minImageCount,
								 info->imageExtent.width,
								 info->imageExtent.height,
								 format, TBM_BO_SCANOUT);

	VK_CHECK(swapchain_tdm->tbm_queue, return VK_ERROR_SURFACE_LOST_KHR,
			 "tbm_surface_queue_create failed.\n");

	if (pthread_mutex_init(&swapchain_tdm->front_mutex, NULL))
		VK_ERROR("pthread_mutex_init front buffer failed\n");
	if (pthread_mutex_init(&swapchain_tdm->free_queue_mutex, NULL))
		VK_ERROR("pthread_mutex_init free queue failed\n");
	if (pthread_cond_init(&swapchain_tdm->free_queue_cond, NULL))
		VK_ERROR("pthread_cond_init free queue failed\n");

	swapchain_tdm->present_mode = info->presentMode;
	swapchain_tdm->tdm_mode = disp_mode->tdm_mode;
	swapchain_tdm->tdm_dpms = TDM_OUTPUT_DPMS_OFF;

	chain->get_buffers = swapchain_tdm_get_buffers;
	chain->deinit = swapchain_tdm_deinit;
	chain->acquire_image = swapchain_tdm_acquire_next_image;
	chain->present_image = swapchain_tdm_queue_present_image;

	return VK_SUCCESS;
}

