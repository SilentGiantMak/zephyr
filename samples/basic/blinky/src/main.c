/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   500

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void main(void)
{
	k_msleep(5000);
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		printk("Device not ready!\n");
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Could not configure the led!\n");
		return;
	}

	int count = 0;
	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		printk("Toggled %d\n",count);
		count++;
		if (ret < 0) {
			printk("LED err!\n");
			return;
		}

		if (count == 10)
		{
			printk("Writing to a random register...\n");
			sys_write32(0x1f,0xF8001000);
		}
		k_msleep(SLEEP_TIME_MS);
	}
}
