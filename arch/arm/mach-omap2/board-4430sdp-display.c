/*
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Modified from mach-omap2/board-4430sdp.c
 *
 * Author: Mythri P K <mythripk@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <plat/display.h>
#include "mux.h"

#define HDMI_GPIO_HPD 60 /* Hot plug pin for HDMI */
#define HDMI_GPIO_LS_OE 41 /* Level shifter for HDMI */

static int sdp4430_panel_enable_hdmi(struct omap_dss_device *dssdev)
{
	int status;
static int once=0;

	if (once > 0)
		return 0;
	once++;
printk(KERN_ERR "sebj - 00 - %d - %s\n", once, __func__);

	/* PAD0_HDMI_HPD_PAD1_HDMI_CEC */
	omap_mux_init_signal("hdmi_hpd",
			OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("hdmi_cec",
			OMAP_PIN_INPUT_PULLUP);
	/* PAD0_HDMI_DDC_SCL_PAD1_HDMI_DDC_SDA */
	omap_mux_init_signal("hdmi_ddc_scl",
			OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("hdmi_ddc_sda",
			OMAP_PIN_INPUT_PULLUP);
printk(KERN_ERR "sebj - 01\n");
	status = gpio_request(HDMI_GPIO_HPD , "hdmi_gpio_hpd");
	if (status) {
		pr_err("Cannot request GPIO %d\n", HDMI_GPIO_HPD);
		return status;
	}
printk(KERN_ERR "sebj - 02\n");
	status = gpio_request(HDMI_GPIO_LS_OE , "hdmi_gpio_ls_oe");
	if (status) {
		pr_err("Cannot request GPIO %d\n", HDMI_GPIO_LS_OE);
		goto error1;
	}
printk(KERN_ERR "sebj - 03\n");
	status = gpio_direction_output(HDMI_GPIO_HPD, 0);
	if (status) {
		pr_err("Cannot set output  GPIO %d\n", HDMI_GPIO_HPD);
		goto error2;
	}
printk(KERN_ERR "sebj - 04\n");
	status = gpio_direction_output(HDMI_GPIO_LS_OE, 0);
	if (status) {
		pr_err("Cannot set output  GPIO %d\n", HDMI_GPIO_LS_OE);
		goto error2;
	}

printk(KERN_ERR "sebj - 05\n");
	/* The value set a pulse */
	gpio_set_value(HDMI_GPIO_HPD, 1);
	gpio_set_value(HDMI_GPIO_LS_OE, 1);
	gpio_set_value(HDMI_GPIO_HPD, 0);
	gpio_set_value(HDMI_GPIO_LS_OE, 0);
	gpio_set_value(HDMI_GPIO_HPD, 1);
	gpio_set_value(HDMI_GPIO_LS_OE, 1);
printk(KERN_ERR "sebj - 06\n");

	return 0;

error2:
	gpio_free(HDMI_GPIO_LS_OE);
error1:
	gpio_free(HDMI_GPIO_HPD);

	return status;
}

static void sdp4430_panel_disable_hdmi(struct omap_dss_device *dssdev)
{
	gpio_free(HDMI_GPIO_LS_OE);
	gpio_free(HDMI_GPIO_HPD);
}

static struct omap_dss_device sdp4430_hdmi_device = {
	.name = "hdmi",
	.driver_name = "hdmi_panel",
	.type = OMAP_DISPLAY_TYPE_HDMI,
	.phy.dpi.data_lines = 24,
	.platform_enable = sdp4430_panel_enable_hdmi,
	.platform_disable = sdp4430_panel_disable_hdmi,
	.channel = OMAP_DSS_CHANNEL_DIGIT,
};

static struct omap_dss_device *sdp4430_dss_devices[] = {
	&sdp4430_hdmi_device,
};

static struct omap_dss_board_info sdp4430_dss_data = {
	.num_devices	= ARRAY_SIZE(sdp4430_dss_devices),
	.devices	= sdp4430_dss_devices,
	.default_device	= &sdp4430_hdmi_device,
};

void __init omap_4430sdp_display_init(void)
{
	omap_display_init(&sdp4430_dss_data);
}
