/*
 * hdmi_omap4_panel.c
 *
 * HDMI library support functions for TI OMAP4 processors.
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com/
 * Authors:	MythriPk <mythripk@ti.com>
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

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <plat/display.h>

#include "dss.h"

static struct {
	struct mutex hdmi_lock;
} hdmi;


static int hdmi_panel_probe(struct omap_dss_device *dssdev)
{
	DSSDBG("ENTER hdmi_panel_probe\n");

	dssdev->panel.config = OMAP_DSS_LCD_TFT |
			OMAP_DSS_LCD_IVS | OMAP_DSS_LCD_IHS;

	/*
	 * Initialize the timings to 1920 * 1080
	 * This is only for framebuffer update not for TV timing setting
	 * Setting TV timing will be done only on enable
	 */
	dssdev->panel.timings.x_res = 1920;
	dssdev->panel.timings.y_res = 1080;

	DSSDBG("hdmi_panel_probe x_res= %d y_res = %d\n",
		dssdev->panel.timings.x_res,
		dssdev->panel.timings.y_res);
	return 0;
}

static void hdmi_panel_remove(struct omap_dss_device *dssdev)
{

}

static int hdmi_panel_enable(struct omap_dss_device *dssdev)
{
	int r = 0;
	DSSDBG("ENTER hdmi_panel_enable\n");

	mutex_lock(&hdmi.hdmi_lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_DISABLED)
		return -EINVAL;

	r = omapdss_hdmi_display_enable(dssdev);
	if (r) {
		DSSERR("failed to power on\n");
		return r;
	}

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	mutex_unlock(&hdmi.hdmi_lock);

	return 0;
}

static void hdmi_panel_disable(struct omap_dss_device *dssdev)
{
	mutex_lock(&hdmi.hdmi_lock);

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE)
		omapdss_hdmi_display_disable(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;

	mutex_unlock(&hdmi.hdmi_lock);
}

static int hdmi_panel_suspend(struct omap_dss_device *dssdev)
{
	mutex_lock(&hdmi.hdmi_lock);

	if (dssdev->state == OMAP_DSS_DISPLAY_DISABLED ||
			dssdev->state == OMAP_DSS_DISPLAY_SUSPENDED)
		return -EINVAL;

	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;

	omapdss_hdmi_display_disable(dssdev);

	mutex_unlock(&hdmi.hdmi_lock);

	return 0;
}

static int hdmi_panel_resume(struct omap_dss_device *dssdev)
{
	mutex_lock(&hdmi.hdmi_lock);

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE)
		return -EINVAL;

	omapdss_hdmi_display_enable(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	mutex_unlock(&hdmi.hdmi_lock);

	return 0;
}

static void hdmi_get_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	mutex_lock(&hdmi.hdmi_lock);

	*timings = dssdev->panel.timings;

	mutex_unlock(&hdmi.hdmi_lock);
}

static void hdmi_set_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	DSSDBG("hdmi_set_timings\n");

	mutex_lock(&hdmi.hdmi_lock);

	dssdev->panel.timings = *timings;

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE) {
		/* turn the hdmi off and on to get new timings to use */
		omapdss_hdmi_display_disable(dssdev);
		omapdss_hdmi_display_enable(dssdev);
	}

	mutex_unlock(&hdmi.hdmi_lock);
}

static struct omap_dss_driver hdmi_driver = {
	.probe		= hdmi_panel_probe,
	.remove		= hdmi_panel_remove,
	.enable		= hdmi_panel_enable,
	.disable	= hdmi_panel_disable,
	.suspend	= hdmi_panel_suspend,
	.resume		= hdmi_panel_resume,
	.get_timings	= hdmi_get_timings,
	.set_timings	= hdmi_set_timings,
	.driver			= {
		.name   = "hdmi_panel",
		.owner  = THIS_MODULE,
	},
};

static int __init hdmi_panel_init(void)
{
	mutex_init(&hdmi.hdmi_lock);

	omap_dss_register_driver(&hdmi_driver);

	return 0;
}

static void __exit hdmi_panel_exit(void)
{
	omap_dss_unregister_driver(&hdmi_driver);

}

module_init(hdmi_panel_init);
module_exit(hdmi_panel_exit);
