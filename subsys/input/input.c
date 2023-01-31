/*
 * Copyright 2023 Google LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(input, CONFIG_INPUT_LOG_LEVEL);

#define INPUT_TYPE_MASK 0x7fff

#define INPUT_TYPE(t) (t & INPUT_TYPE_MASK)

#define INPUT_SYN_IS_SET(v) (v & INPUT_EV_SYN)

#ifdef CONFIG_INPUT_MODE_THREAD

K_MSGQ_DEFINE(input_msgq, sizeof(struct input_event),
	      CONFIG_INPUT_QUEUE_MAX_MSGS, 4);

#endif

static void input_process(struct input_event *evt)
{
	bool sync = INPUT_SYN_IS_SET(evt->type);

	evt->type = INPUT_TYPE(evt->type);

	STRUCT_SECTION_FOREACH(input_listener, listener) {
		if (listener->dev == NULL || listener->dev == evt->dev) {
			listener->callback(evt, sync);
		}
	}
}

bool input_queue_empty(void)
{
#ifdef CONFIG_INPUT_MODE_THREAD
	if (k_msgq_num_used_get(&input_msgq) > 0) {
		return false;
	}
#endif
	return true;
}

int input_report(const struct device *dev,
		 uint16_t type, uint16_t code, int32_t value, bool sync,
		 k_timeout_t timeout)
{
	struct input_event evt = {
		.dev = dev,
		.type = type,
		.code = code,
		.value = value,
	};

	if (type & ~INPUT_TYPE_MASK) {
		return -EINVAL;
	}

	if (sync) {
		evt.type |= INPUT_EV_SYN;
	}

#ifdef CONFIG_INPUT_MODE_THREAD
	return k_msgq_put(&input_msgq, &evt, timeout);
#else
	input_process(&evt);
	return 0;
#endif
}

#ifdef CONFIG_INPUT_MODE_THREAD

static void input_thread(void)
{
	struct input_event evt;
	int ret;

	while (true) {
		ret = k_msgq_get(&input_msgq, &evt, K_FOREVER);
		if (ret) {
			LOG_ERR("k_msgq_get error: %d", ret);
			continue;
		}

		input_process(&evt);
	}
}

#define INPUT_THREAD_PRIORITY \
	COND_CODE_1(CONFIG_INPUT_THREAD_PRIORITY_OVERRIDE, \
		    (CONFIG_INPUT_THREAD_PRIORITY), (K_LOWEST_APPLICATION_THREAD_PRIO))

K_THREAD_DEFINE(input,
		CONFIG_INPUT_THREAD_STACK_SIZE,
		input_thread,
		NULL, NULL, NULL,
		INPUT_THREAD_PRIORITY, 0, 0);

#endif /* CONFIG_INPUT_MODE_THREAD */
