// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2019 Intel Corporation. All rights reserved.
//
// Author: Marcin Rajwa <marcin.rajwa@linux.intel.com>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <malloc.h>
#include <cmocka.h>

#include <sof/lib/alloc.h>
#include <sof/lib/notifier.h>
#include <sof/lib/pm_runtime.h>
#include <sof/audio/component.h>
#include <sof/drivers/timer.h>
#include <sof/schedule/edf_schedule.h>
#include <sof/schedule/ll_schedule.h>
#include <sof/schedule/schedule.h>
#include <mock_trace.h>
#include <sof/lib/clk.h>

TRACE_IMPL()


void *_balloc(uint32_t flags, uint32_t caps, size_t bytes,
	      uint32_t alignment)
{
	(void)flags;
	(void)caps;

	return malloc(bytes);
}

void *_zalloc(enum mem_zone zone, uint32_t flags, uint32_t caps, size_t bytes)
{
	(void)zone;
	(void)flags;
	(void)caps;

	return calloc(bytes, 1);
}

void rfree(void *ptr)
{
	free(ptr);
}

void *_brealloc(void *ptr, uint32_t flags, uint32_t caps, size_t bytes,
		uint32_t alignment)
{
	(void)flags;
	(void)caps;

	return realloc(ptr, bytes);
}

void pipeline_xrun(struct pipeline *p, struct comp_dev *dev, int32_t bytes)
{
}

int comp_set_state(struct comp_dev *dev, int cmd)
{
	return 0;
}

int schedule_task_init(struct task *task, uint16_t type, uint16_t priority,
		       enum task_state (*run)(void *data), void *data,
		       uint16_t core, uint32_t flags)
{
	return 0;
}

int schedule_task_init_edf(struct task *task, const struct task_ops *ops,
			   void *data, uint16_t core, uint32_t flags)
{
	return 0;
}

int schedule_task_init_ll(struct task *task, uint16_t type, uint16_t priority,
			  enum task_state (*run)(void *data), void *data,
			  uint16_t core, uint32_t flags)
{
	return 0;
}

void __panic(uint32_t p, char *filename, uint32_t linenum)
{
	(void)p;
	(void)filename;
	(void)linenum;
}

uint64_t platform_timer_get(struct timer *timer)
{
	(void)timer;

	return 0;
}

uint64_t arch_timer_get_system(struct timer *timer)
{
	(void)timer;

	return 0;
}

uint64_t clock_ms_to_ticks(int clock, uint64_t ms)
{
	(void)clock;
	(void)ms;

	return 0;
}

struct schedulers **arch_schedulers_get(void)
{
	return NULL;
}

void pm_runtime_enable(enum pm_runtime_context context, uint32_t index)
{
	(void)context;
	(void)index;
}

void pm_runtime_disable(enum pm_runtime_context context, uint32_t index)
{
	(void)context;
	(void)index;
}



