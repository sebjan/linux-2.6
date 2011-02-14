/*
 * hdmi.c
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com/
 * Authors: Yong Zhi
 *	Mythri pk <mythripk@ti.com>
 *
 * HDMI interface DSS driver setting for TI's OMAP4 family of processor.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define DSS_SUBSYS_NAME "HDMI"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <plat/display.h>
#include <plat/cpu.h>

#include "dss.h"

static struct {
	struct mutex lock;
	struct omap_display_platform_data *pdata;
	struct platform_device *pdev;
	struct resource *hdmi_mem;
} hdmi;

#define FLD_GET(val, start, end) (((val) & FLD_MASK(start, end)) >> (end))
#define FLD_MOD(orig, val, start, end) \
	(((orig) & ~FLD_MASK(start, end)) | FLD_VAL(val, start, end))

int hdmi_init_display(struct omap_dss_device *dssdev)
{
	DSSDBG("init_display\n");

	return 0;
}

void omapdss_hdmi_compute_pll(int clkin, int phy,
	int n, struct hdmi_pll_info *pi)
{
	int refclk;
	u32 mf;

	/*
	 * Input clock is predivided by N + 1
	 * out put of which is reference clk
	 */
	refclk = clkin / (n + 1);
	pi->regn = n;

	/*
	 * multiplier is pixel_clk/ref_clk
	 * Multiplying by 100 to avoid fractional part removal
	 */
	pi->regm = (phy * 100/(refclk))/100;
	pi->regm2 = 1;

	/*
	 * fractional multiplier is remainder of the difference between
	 * multiplier and actual phy(required pixel clock thus should be
	 * multiplied by 2^18(262144) divided by the reference clock
	 */
	mf = (phy - pi->regm * refclk) * 262144;
	pi->regmf = mf/(refclk);

	/*
	 * Dcofreq should be set to 1 if required pixel clock
	 * is greater than 1000MHz
	 */
	pi->dcofreq = phy > 1000 * 100;
	pi->regsd = ((pi->regm * clkin / 10) / ((n + 1) * 250) + 5) / 10;

	DSSDBG("M = %d Mf = %d\n", pi->regm, pi->regmf);
	DSSDBG("range = %d sd = %d\n", pi->dcofreq, pi->regsd);
}

static void hdmi_enable_clocks(int enable)
{
	if (enable)
		dss_clk_enable(DSS_CLK_ICK | DSS_CLK_FCK |
				DSS_CLK_SYSCK | DSS_CLK_VIDFCK);
	else
		dss_clk_disable(DSS_CLK_ICK | DSS_CLK_FCK |
				DSS_CLK_SYSCK | DSS_CLK_VIDFCK);
}

static int hdmi_power_on(struct omap_dss_device *dssdev)
{
	hdmi_enable_clocks(1);
	dispc_enable_channel(OMAP_DSS_CHANNEL_DIGIT, 0);

	return 0;
}

int omapdss_hdmi_dispc_setting(struct omap_dss_device *dssdev)
{
	/* these settings are independent of overlays */
	dss_select_hdmi_venc(1);

	/* bypass TV gamma table */
	dispc_enable_gamma_table(0);

	/* tv size */
	dispc_set_digit_size(dssdev->panel.timings.x_res,
			dssdev->panel.timings.y_res);
	dispc_enable_channel(OMAP_DSS_CHANNEL_DIGIT, 1);

	return 0;
}

static void hdmi_power_off(struct omap_dss_device *dssdev)
{
	dispc_enable_channel(OMAP_DSS_CHANNEL_DIGIT, 0);

	if (dssdev->platform_disable)
		dssdev->platform_disable(dssdev);

	hdmi_enable_clocks(0);
}

int omapdss_hdmi_display_enable(struct omap_dss_device *dssdev)
{
	int r = 0;

	DSSDBG("ENTER hdmi_display_enable\n");

	r = omap_dss_start_device(dssdev);
	if (r) {
		DSSDBG("failed to start device\n");
		return r;
	}

	if (dssdev->platform_enable)
		dssdev->platform_enable(dssdev);

	r = hdmi_power_on(dssdev);
	if (r) {
		DSSERR("failed to power on device\n");
		return r;
	}

	return 0;
}

void omapdss_hdmi_display_disable(struct omap_dss_device *dssdev)
{
	DSSDBG("Enter hdmi_display_disable\n");

	omap_dss_stop_device(dssdev);
	hdmi_power_off(dssdev);
}

int omapdss_hdmi_display_suspend(struct omap_dss_device *dssdev)
{
	DSSDBG("hdmi_display_suspend\n");

	mutex_lock(&hdmi.lock);
	omap_dss_stop_device(dssdev);
	hdmi_power_off(dssdev);
	mutex_unlock(&hdmi.lock);

	return 0;
}

int omapdss_hdmi_display_resume(struct omap_dss_device *dssdev)
{
	int r = 0;

	DSSDBG("hdmi_display_resume\n");

	mutex_lock(&hdmi.lock);
	r = omap_dss_start_device(dssdev);
	if (r) {
		DSSERR("failed to start device\n");
		goto err;
	}

	if (dssdev->platform_enable)
		dssdev->platform_enable(dssdev);

	r = hdmi_power_on(dssdev);
	if (r) {
		DSSERR("failed to power on device\n");
		goto err;
	}

	mutex_unlock(&hdmi.lock);

	return 0;
err:
	mutex_unlock(&hdmi.lock);

	return r;
}

void omapdss_hdmi_base_address(int *hdmi_start, int *size)
{
	*hdmi_start = hdmi.hdmi_mem->start;
	*size = resource_size(hdmi.hdmi_mem);
}

/* HDMI HW IP initialisation */
static int omap_hdmihw_probe(struct platform_device *pdev)
{
	hdmi.pdata = pdev->dev.platform_data;
	hdmi.pdev = pdev;
	hdmi.hdmi_mem = platform_get_resource(hdmi.pdev, IORESOURCE_MEM, 0);
	if (!hdmi.hdmi_mem) {
		DSSERR("can't get IORESOURCE_MEM HDMI\n");
		return -EINVAL;
	}

	return 0;
}

static int omap_hdmihw_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver omap_hdmihw_driver = {
	.probe          = omap_hdmihw_probe,
	.remove         = omap_hdmihw_remove,
	.driver         = {
		.name   = "omap_hdmi",
		.owner  = THIS_MODULE,
	},
};

int hdmi_init_platform_driver(void)
{
	return platform_driver_register(&omap_hdmihw_driver);
}

void hdmi_uninit_platform_driver(void)
{
	return platform_driver_unregister(&omap_hdmihw_driver);
}
