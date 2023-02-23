/*
 * Copyright 2023 Google LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>

static void input_cb(struct input_event *evt)
{
	printk("input event: dev=%-16s %3s type=%2x code=%3d value=%d\n",
	       evt->dev->name,
	       evt->sync ? "SYN" : "",
	       evt->type,
	       evt->code,
	       evt->value);
}
INPUT_LISTENER_CB_DEFINE(NULL, input_cb);

void main(void)
{
	printk("Input sample started\n");
}
