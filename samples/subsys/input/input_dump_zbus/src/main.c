/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>
#include <zephyr/zbus/zbus.h>

ZBUS_CHAN_DECLARE(sys_chan_input);

void input_listener(const struct zbus_channel *chan)
{
	const struct input_event *evt = zbus_chan_const_msg(chan);

	printk("input event: dev=%-16s %3s type=%2x code=%3d value=%d\n", evt->dev->name,
	       evt->sync ? "SYN" : "", evt->type, evt->code, evt->value);
}

ZBUS_LISTENER_DEFINE(input_sample_lis, input_listener);

void main(void)
{
	zbus_chan_add_obs(&sys_chan_input, &input_sample_lis, K_FOREVER);

	printk("Input zbus sample started\n");
}
