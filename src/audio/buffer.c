// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2016 Intel Corporation. All rights reserved.
//
// Author: Liam Girdwood <liam.r.girdwood@linux.intel.com>
//         Keyon Jie <yang.jie@linux.intel.com>

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <sof/sof.h>
#include <sof/lock.h>
#include <sof/list.h>
#include <sof/stream.h>
#include <sof/alloc.h>
#include <sof/debug.h>
#include <sof/ipc.h>
#include <platform/timer.h>
#include <platform/platform.h>
#include <sof/audio/component.h>
#include <sof/audio/pipeline.h>
#include <sof/audio/buffer.h>

/* create a new component in the pipeline */
struct comp_buffer *buffer_new(struct sof_ipc_buffer *desc)
{
	struct comp_buffer *buffer;

	trace_buffer("buffer_new()");

	/* validate request */
	if (desc->size == 0 || desc->size > HEAP_BUFFER_SIZE) {
		trace_buffer_error("buffer_new() error: "
				   "new size = %u is invalid", desc->size);
		return NULL;
	}

	/* allocate new buffer */
	buffer = rzalloc(RZONE_RUNTIME, SOF_MEM_CAPS_RAM, sizeof(*buffer));
	if (!buffer) {
		trace_buffer_error("buffer_new() error: "
				   "could not alloc structure");
		return NULL;
	}

	buffer->addr = rballoc(RZONE_BUFFER, desc->caps, desc->size);
	if (!buffer->addr) {
		rfree(buffer);
		trace_buffer_error("buffer_new() error: "
				   "could not alloc size = %u "
				   "bytes of type = %u",
				   desc->size, desc->caps);
		return NULL;
	}

	assert(!memcpy_s(&buffer->ipc_buffer, sizeof(buffer->ipc_buffer),
		       desc, sizeof(*desc)));

	buffer_init(buffer, desc->size);

	spinlock_init(&buffer->lock);

	return buffer;
}

int buffer_resize(struct comp_buffer *buffer, uint32_t size)
{
	void *new_ptr = NULL;
	struct sof_ipc_buffer *desc = NULL;

	/* validate request */
	if (size == 0 || size > HEAP_BUFFER_SIZE) {
		trace_buffer_error("resize error: size = %u is invalid", size);
		return -EINVAL;
	}

	desc = &buffer->ipc_buffer;
	if (!desc) {
		trace_buffer_error("resize error: invalid buffer desc");
		return -EINVAL;
	}

	new_ptr = rbrealloc(buffer->addr, RZONE_BUFFER, desc->caps, size);

	if (!new_ptr) {
		trace_buffer_error("resize error: can't alloc %u bytes type %u",
				   desc->size, desc->caps);
		return -ENOMEM;
	}

	buffer->addr = new_ptr;
	desc->size = size;

	buffer_init(buffer, desc->size);

	return 0;
}

/* free component in the pipeline */
void buffer_free(struct comp_buffer *buffer)
{
	trace_buffer("buffer_free()");

	list_item_del(&buffer->source_list);
	list_item_del(&buffer->sink_list);
	rfree(buffer->addr);
	rfree(buffer);
}

void comp_update_buffer_produce(struct comp_buffer *buffer, uint32_t bytes)
{
	uint32_t flags;
	uint32_t head = bytes;
	uint32_t tail = 0;

	/* return if no bytes */
	if (!bytes) {
		trace_buffer("comp_update_buffer_produce(), "
			     "no bytes to produce");
		return;
	}

	spin_lock_irq(&buffer->lock, flags);

	/* calculate head and tail size for dcache circular wrap ops */
	if (buffer->w_ptr + bytes > buffer->end_addr) {
		head = buffer->end_addr - buffer->w_ptr;
		tail = bytes - head;
	}

	/*
	 * new data produce, handle consistency for buffer and cache:
	 * 1. source(DMA) --> buffer --> sink(non-DMA): invalidate cache.
	 * 2. source(non-DMA) --> buffer --> sink(DMA): write back to memory.
	 * 3. source(DMA) --> buffer --> sink(DMA): do nothing.
	 * 4. source(non-DMA) --> buffer --> sink(non-DMA): do nothing.
	 */
	if (buffer->source->is_dma_connected &&
	    !buffer->sink->is_dma_connected) {
		/* need invalidate cache for sink component to use */
		dcache_invalidate_region(buffer->w_ptr, head);
		if (tail)
			dcache_invalidate_region(buffer->addr, tail);
	} else if (!buffer->source->is_dma_connected &&
		   buffer->sink->is_dma_connected) {
		/* need write back to memory for sink component to use */
		dcache_writeback_region(buffer->w_ptr, head);
		if (tail)
			dcache_writeback_region(buffer->addr, tail);
	}

	buffer->w_ptr += bytes;

	/* check for pointer wrap */
	if (buffer->w_ptr >= buffer->end_addr)
		buffer->w_ptr = buffer->addr +
			(buffer->w_ptr - buffer->end_addr);

	/* calculate available bytes */
	if (buffer->r_ptr < buffer->w_ptr)
		buffer->avail = buffer->w_ptr - buffer->r_ptr;
	else if (buffer->r_ptr == buffer->w_ptr)
		buffer->avail = buffer->size; /* full */
	else
		buffer->avail = buffer->size - (buffer->r_ptr - buffer->w_ptr);

	/* calculate free bytes */
	buffer->free = buffer->size - buffer->avail;

	if (buffer->cb && buffer->cb_type & BUFF_CB_TYPE_PRODUCE)
		buffer->cb(buffer->cb_data, bytes);

	spin_unlock_irq(&buffer->lock, flags);

	tracev_buffer("comp_update_buffer_produce(), ((buffer->avail << 16) | "
		      "buffer->free) = %08x, ((buffer->ipc_buffer.comp.id << "
		      "16) | buffer->size) = %08x",
		      (buffer->avail << 16) | buffer->free,
		      (buffer->ipc_buffer.comp.id << 16) | buffer->size);
	tracev_buffer("comp_update_buffer_produce(), ((buffer->r_ptr - buffer"
		      "->addr) << 16 | (buffer->w_ptr - buffer->addr)) = %08x",
		      (buffer->r_ptr - buffer->addr) << 16 |
		      (buffer->w_ptr - buffer->addr));
}

void comp_update_buffer_consume(struct comp_buffer *buffer, uint32_t bytes)
{
	uint32_t flags;

	/* return if no bytes */
	if (!bytes) {
		trace_buffer("comp_update_buffer_consume(), "
			     "no bytes to consume");
		return;
	}

	spin_lock_irq(&buffer->lock, flags);

	buffer->r_ptr += bytes;

	/* check for pointer wrap */
	if (buffer->r_ptr >= buffer->end_addr)
		buffer->r_ptr = buffer->addr +
			(buffer->r_ptr - buffer->end_addr);

	/* calculate available bytes */
	if (buffer->r_ptr < buffer->w_ptr)
		buffer->avail = buffer->w_ptr - buffer->r_ptr;
	else if (buffer->r_ptr == buffer->w_ptr)
		buffer->avail = 0; /* empty */
	else
		buffer->avail = buffer->size - (buffer->r_ptr - buffer->w_ptr);

	/* calculate free bytes */
	buffer->free = buffer->size - buffer->avail;

	if (buffer->sink->is_dma_connected &&
	    !buffer->source->is_dma_connected)
		dcache_writeback_region(buffer->r_ptr, bytes);

	if (buffer->cb && buffer->cb_type & BUFF_CB_TYPE_CONSUME)
		buffer->cb(buffer->cb_data, bytes);

	spin_unlock_irq(&buffer->lock, flags);

	tracev_buffer("comp_update_buffer_consume(), "
		      "(buffer->avail << 16) | buffer->free = %08x, "
		      "(buffer->ipc_buffer.comp.id << 16) | buffer->size = "
		      " %08x, (buffer->r_ptr - buffer->addr) << 16 | "
		      "(buffer->w_ptr - buffer->addr)) = %08x",
		      (buffer->avail << 16) | buffer->free,
		      (buffer->ipc_buffer.comp.id << 16) | buffer->size,
		      (buffer->r_ptr - buffer->addr) << 16 |
		      (buffer->w_ptr - buffer->addr));
}
