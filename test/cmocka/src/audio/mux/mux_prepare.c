// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2019 Intel Corporation. All rights reserved.
//
// Author: Daniel Bogdzia <danielx.bogdzia@linux.intel.com>
//         Janusz Jankowski <janusz.jankowski@linux.intel.com>

#include <sof/audio/component.h>
#include <sof/audio/mux.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <cmocka.h>

struct sof *sof_get(void);

struct test_data {
	struct comp_dev *dev;
	struct comp_data *cd;
};

static int setup_group(void **state)
{
	sys_comp_init(sof_get());
	sys_comp_mux_init();

	return 0;
}

static int setup_test_case(void **state)
{
	struct test_data *td = malloc(sizeof(struct test_data));
	struct sof_ipc_comp_process ipc = {
		.comp = {
			.hdr = {
				.size = sizeof(struct sof_ipc_comp_process)
			},
			.type = SOF_COMP_MUX
		},
		.config = {
			.hdr = {
				.size = sizeof(struct sof_ipc_comp_config)
			}
		}
	};

	td->dev = comp_new((struct sof_ipc_comp *)&ipc);
	if (!td->dev)
		return -EINVAL;

	td->cd = (struct comp_data *)td->dev->private;

	*state = td;

	return 0;
}

static int teardown_test_case(void **state)
{
	struct test_data *td = *state;

	comp_free(td->dev);
	free(td);

	return 0;
}

#if CONFIG_FORMAT_FLOAT
static void test_mux_prepare_invalid_float(void **state)
{
	struct test_data *td = *state;

	/* set frame format value to unsupported value */
	td->cd->config.frame_format = SOF_IPC_FRAME_FLOAT;

	assert_int_equal(comp_prepare(td->dev), -EINVAL);
}
#endif /* CONFIG_FORMAT_FLOAT */

#if CONFIG_FORMAT_S16LE
static void test_mux_prepare_valid_s16le(void **state)
{
	struct test_data *td = *state;

	td->cd->config.frame_format = SOF_IPC_FRAME_S16_LE;

	assert_int_equal(comp_prepare(td->dev), 0);
}
#endif /* CONFIG_FORMAT_S16LE */

#if CONFIG_FORMAT_S24LE
static void test_mux_prepare_valid_s24_4le(void **state)
{
	struct test_data *td = *state;

	td->cd->config.frame_format = SOF_IPC_FRAME_S24_4LE;

	assert_int_equal(comp_prepare(td->dev), 0);
}
#endif /* CONFIG_FORMAT_S24LE */

#if CONFIG_FORMAT_S32LE
static void test_mux_prepare_valid_s32le(void **state)
{
	struct test_data *td = *state;

	td->cd->config.frame_format = SOF_IPC_FRAME_S32_LE;

	assert_int_equal(comp_prepare(td->dev), 0);
}
#endif /* CONFIG_FORMAT_S32LE */

#define TEST_CASE(name) \
	cmocka_unit_test_setup_teardown(name, \
					setup_test_case, \
					teardown_test_case)

int main(void)
{
	const struct CMUnitTest tests[] = {
#if CONFIG_FORMAT_FLOAT
		TEST_CASE(test_mux_prepare_invalid_float),
#endif /* CONFIG_FORMAT_FLOAT */
#if CONFIG_FORMAT_S16LE
		TEST_CASE(test_mux_prepare_valid_s16le),
#endif /* CONFIG_FORMAT_S16LE */
#if CONFIG_FORMAT_S24LE
		TEST_CASE(test_mux_prepare_valid_s24_4le),
#endif /* CONFIG_FORMAT_S24LE */
#if CONFIG_FORMAT_S32LE
		TEST_CASE(test_mux_prepare_valid_s32le),
#endif /* CONFIG_FORMAT_S32LE */
	};

	cmocka_set_message_output(CM_OUTPUT_TAP);

	return cmocka_run_group_tests(tests, setup_group, NULL);
}
