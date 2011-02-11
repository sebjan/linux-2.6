/*
 * hdmi_omap4_panel.c
 *
 * HDMI library support functions for TI OMAP4 processors.
 *
 * Copyright (C) 2010-2011 Texas Instruments
 * Authors:	MythriPk <mythripk@ti.com>
 *		Yong Zhi <y-zhi@ti.com>
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
#include <linux/err.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/seq_file.h>

#include "hdmi_omap4_panel.h"

static struct {
	void __iomem *base_wp;		/* HDMI wrapper */
	int code;
	int mode;
	struct hdmi_config cfg;
	struct mutex hdmi_lock;
} hdmi;

static inline void hdmi_write_reg(u32 base, u16 idx, u32 val)
{
	__raw_writel(val, hdmi.base_wp + base + idx);
}

static inline u32 hdmi_read_reg(u32 base, u16 idx)
{
	u32 l;
	l = __raw_readl(hdmi.base_wp + base + idx);
	return l;
}

static inline int hdmi_wait_for_bit_change(const u32 ins,
	u32 idx, int b2, int b1, int val)
{
	int t = 0;
	while (val != FLD_GET(hdmi_read_reg(ins, idx), b2, b1)) {
		udelay(1);
		if (t++ > 1000)
			return !val;
	}
	return val;
}

static int hdmi_pll_init(int refsel, int dcofreq,
		struct hdmi_pll_info *fmt, u16 sd)
{
	u32 r;

	/* PLL start always use manual mode */
	REG_FLD_MOD(HDMI_PLLCTRL, PLLCTRL_PLL_CONTROL, 0x0, 0, 0);

	r = hdmi_read_reg(HDMI_PLLCTRL, PLLCTRL_CFG1);
	r = FLD_MOD(r, fmt->regm, 20, 9); /* CFG1_PLL_REGM */
	r = FLD_MOD(r, fmt->regn, 8, 1);  /* CFG1_PLL_REGN */

	hdmi_write_reg(HDMI_PLLCTRL, PLLCTRL_CFG1, r);

	r = hdmi_read_reg(HDMI_PLLCTRL, PLLCTRL_CFG2);

	r = FLD_MOD(r, 0x0, 12, 12); /* PLL_HIGHFREQ divide by 2 */
	r = FLD_MOD(r, 0x1, 13, 13); /* PLL_REFEN */
	r = FLD_MOD(r, 0x0, 14, 14); /* PHY_CLKINEN de-assert during locking */

	if (dcofreq) {
		/* divider programming for frequency beyond 1000Mhz */
		REG_FLD_MOD(HDMI_PLLCTRL, PLLCTRL_CFG3, sd, 17, 10);
		r = FLD_MOD(r, 0x4, 3, 1); /* 1000MHz and 2000MHz */
	} else {
		r = FLD_MOD(r, 0x2, 3, 1); /* 500MHz and 1000MHz */
	}

	hdmi_write_reg(HDMI_PLLCTRL, PLLCTRL_CFG2, r);

	r = hdmi_read_reg(HDMI_PLLCTRL, PLLCTRL_CFG4);
	r = FLD_MOD(r, fmt->regm2, 24, 18);
	r = FLD_MOD(r, fmt->regmf, 17, 0);

	hdmi_write_reg(HDMI_PLLCTRL, PLLCTRL_CFG4, r);

	/* go now */
	REG_FLD_MOD(HDMI_PLLCTRL, PLLCTRL_PLL_GO, 0x1, 0, 0);

	/* wait for bit change */
	if (hdmi_wait_for_bit_change(HDMI_PLLCTRL,
		PLLCTRL_PLL_GO, 0, 0, 1) != 1) {
		pr_err("Pll Go bit not set\n");
		return -ETIMEDOUT;
	}

	/* Wait till the lock bit is set in PLL status */
	if (hdmi_wait_for_bit_change(HDMI_PLLCTRL,
		PLLCTRL_PLL_STATUS, 1, 1, 1) != 1) {
		pr_err("Pll Go bit not set\n");
		pr_warning("HDMI: cannot lock PLL\n");
		pr_warning("CFG1 0x%x\n",
			hdmi_read_reg(HDMI_PLLCTRL, PLLCTRL_CFG1));
		pr_warning("CFG2 0x%x\n",
			hdmi_read_reg(HDMI_PLLCTRL, PLLCTRL_CFG2));
		pr_warning("CFG4 0x%x\n",
			hdmi_read_reg(HDMI_PLLCTRL, PLLCTRL_CFG4));
		return -ETIMEDOUT;
	}

	pr_info("PLL locked!\n");

	return 0;
}

static int hdmi_pll_reset(void)
{
	/* SYSRESET  controlled by power FSM */
	REG_FLD_MOD(HDMI_PLLCTRL, PLLCTRL_PLL_CONTROL, 0x0, 3, 3);

	/* READ 0x0 reset is in progress */
	if (hdmi_wait_for_bit_change(HDMI_PLLCTRL,
		PLLCTRL_PLL_STATUS, 0, 0, 1) != 1) {
		pr_err("Failed to sysreset PLL\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int hdmi_phy_init(u32 w1, u32 phy)
{
	int r = 0;

	/*
	 * wait till PHY_PWR_STATUS=LDOON
	 * HDMI_PHYPWRCMD_LDOON = 1
	 */
	r = hdmi_wait_phy_pwr(1);
	if (r)
		return r;

	/* wait till PHY_PWR_STATUS=TXON */
	r = hdmi_wait_phy_pwr(2);
	if (r)
		return r;

	/*
	 * Read address 0 in order to get the SCP reset done completed
	 * Dummy access performed to make sure reset is done
	 */
	hdmi_read_reg(phy, HDMI_TXPHY_TX_CTRL);

	/*
	 * Write to phy address 0 to configure the clock
	 * use HFBITCLK write HDMI_TXPHY_TX_CONTROL_FREQOUT field
	 */
	REG_FLD_MOD(phy, HDMI_TXPHY_TX_CTRL, 0x1, 31, 30);

	/* write to phy address 1 to start HDMI line (TXVALID and TMDSCLKEN) */
	hdmi_write_reg(phy, HDMI_TXPHY_DIGITAL_CTRL, 0xF0000000);

	/* setup max LDO voltage */
	REG_FLD_MOD(phy, HDMI_TXPHY_POWER_CTRL, 0xB, 3, 0);

	/*  write to phy address 3 to change the polarity control */
	REG_FLD_MOD(phy, HDMI_TXPHY_PAD_CFG_CTRL, 0x1, 27, 27);

	return 0;
}

int hdmi_pll_program(struct hdmi_pll_info *fmt)
{
	u32 r = 0;
	int refsel;

	int pllpwrwait;

	/* wait for wrapper reset */
	hdmi_wait_softreset();

	/* power off PLL */
	pllpwrwait = HDMI_PLLPWRCMD_ALLOFF;
	r = hdmi_wait_pll_pwr(pllpwrwait);
	if (r)
		return r;

	/* power on PLL */
	pllpwrwait = HDMI_PLLPWRCMD_BOTHON_ALLCLKS;
	r = hdmi_wait_pll_pwr(pllpwrwait);
	if (r)
		return r;

	hdmi_pll_reset();

	refsel = 0x3; /* select SYSCLK reference */

	r = hdmi_pll_init(refsel, fmt->dcofreq, fmt, fmt->regsd);
	if (r)
		return r;

	return 0;
}

static int hdmi_phy_off(void)
{
	/*
	 * Wait till PHY_PWR_STATUS=OFF
	 * HDMI_PHYPWRCMD_OFF = 0
	 */
	hdmi_wait_phy_pwr(0);

	return 0;
}

int hdmi_core_ddc_edid(u8 *pEDID, int ext)
{
	u32 i, j, l;
	char checksum = 0;
	u32 sts = HDMI_CORE_DDC_STATUS;
	u32 ins = HDMI_CORE_SYS;
	u32 offset = 0;

	/* Turn on CLK for DDC */
	REG_FLD_MOD(HDMI_CORE_AV, HDMI_CORE_AV_DPD, 0x7, 2, 0);

	/* DDC needs time to stablize so Wait. Do not optimize*/
	mdelay(10);

	if (!ext) {
		/* Clk SCL Devices */
		REG_FLD_MOD(ins, HDMI_CORE_DDC_CMD, 0xA, 3, 0);

		/* HDMI_CORE_DDC_STATUS_IN_PROG No timer needed */
		if (hdmi_wait_for_bit_change(ins, sts, 4, 4, 0) != 0) {
			pr_err("Failed to program DDC\n");
			return -ETIMEDOUT;
		}

		/* Clear FIFO */
		REG_FLD_MOD(ins, HDMI_CORE_DDC_CMD, 0x9, 3, 0);

		/* HDMI_CORE_DDC_STATUS_IN_PROG */
		if (hdmi_wait_for_bit_change(ins, sts, 4, 4, 0) != 0) {
			pr_err("Failed to program DDC\n");
			return -ETIMEDOUT;
		}

	} else {
		if (ext % 2 != 0)
			offset = 0x80;
	}

	/* Load Segment Address Register */
	REG_FLD_MOD(ins, HDMI_CORE_DDC_SEGM, ext/2, 7, 0);

	/* Load Slave Address Register */
	REG_FLD_MOD(ins, HDMI_CORE_DDC_ADDR, 0xA0 >> 1, 7, 1);

	/* Load Offset Address Register */
	REG_FLD_MOD(ins, HDMI_CORE_DDC_OFFSET, offset, 7, 0);

	/* Load Byte Count */
	REG_FLD_MOD(ins, HDMI_CORE_DDC_COUNT1, 0x80, 7, 0);
	REG_FLD_MOD(ins, HDMI_CORE_DDC_COUNT2, 0x0, 1, 0);

	/* Set DDC_CMD */
	if (ext)
		REG_FLD_MOD(ins, HDMI_CORE_DDC_CMD, 0x4, 3, 0);
	else
		REG_FLD_MOD(ins, HDMI_CORE_DDC_CMD, 0x2, 3, 0);

	/*
	 * Do not optimize this part of the code, seems
	 * DDC bus needs some time to get stabilized
	 */
	l = hdmi_read_reg(ins, sts);

	/* HDMI_CORE_DDC_STATUS_BUS_LOW */
	if (FLD_GET(l, 6, 6) == 1) {
		pr_warning("I2C Bus Low?\n");
		return -EIO;
	}
	/* HDMI_CORE_DDC_STATUS_NO_ACK */
	if (FLD_GET(l, 5, 5) == 1) {
		pr_warning("I2C No Ack\n");
		return -EIO;
	}

	i = ext * 128;
	j = 0;
	while (((FLD_GET(hdmi_read_reg(ins, sts), 4, 4) == 1) ||
		(FLD_GET(hdmi_read_reg(ins, sts), 2, 2) == 0)) && j < 128) {
		if (FLD_GET(hdmi_read_reg(ins, sts), 2, 2) == 0) {
			/* FIFO not empty */
			pEDID[i++] = FLD_GET(
				hdmi_read_reg(ins, HDMI_CORE_DDC_DATA), 7, 0);
			j++;
		}
	}

	for (j = 0; j < 128; j++)
		checksum += pEDID[j];

	if (checksum != 0) {
		pr_err("E-EDID checksum failed!!\n");
		return -EIO;
	}

	return 0;
}

int read_edid(u8 *pEDID, u16 max_length)
{
	int r = 0, n = 0, i = 0;
	int max_ext_blocks = (max_length / 128) - 1;

	r = hdmi_core_ddc_edid(pEDID, 0);
	if (r) {
		return -EIO;
	} else {
		n = pEDID[0x7e];

		/*
		 * README: need to comply with max_length set by the caller.
		 * Better implementation should be to allocate necessary
		 * memory to store EDID according to nb_block field found
		 * in first block
		 */

		if (n > max_ext_blocks)
			n = max_ext_blocks;

		for (i = 1; i <= n; i++) {
			r = hdmi_core_ddc_edid(pEDID, i);
			if (r)
				return -EIO;
		}
	}
	return 0;
}

static inline void print_omap_video_timings(struct omap_video_timings *timings)
{
	pr_info("Timing Info:\n");
	pr_info("pixel_clk = %d\n", timings->pixel_clock);
	pr_info("  x_res     = %d\n", timings->x_res);
	pr_info("  y_res     = %d\n", timings->y_res);
	pr_info("  hfp       = %d\n", timings->hfp);
	pr_info("  hsw       = %d\n", timings->hsw);
	pr_info("  hbp       = %d\n", timings->hbp);
	pr_info("  vfp       = %d\n", timings->vfp);
	pr_info("  vsw       = %d\n", timings->vsw);
	pr_info("  vbp       = %d\n", timings->vbp);
}

static int get_timings_index(void)
{
	int code;

	if (hdmi.mode == 0)
		code = code_vesa[hdmi.code];
	else
		code = code_cea[hdmi.code];

	if (code == -1)	{
		code = 9;
		hdmi.code = 16;
		hdmi.mode = 1;
	}
	return code;
}

static struct hdmi_cm hdmi_get_code(struct omap_video_timings *timing)
{
	int i = 0, code = -1, temp_vsync = 0, temp_hsync = 0;
	int timing_vsync = 0, timing_hsync = 0;
	struct omap_video_timings temp;
	struct hdmi_cm cm = {-1};
	pr_info("hdmi_get_code\n");

	for (i = 0; i < OMAP_HDMI_TIMINGS_NB; i++) {
		temp = cea_vesa_timings[i].timings;
		if ((temp.pixel_clock == timing->pixel_clock) &&
			(temp.x_res == timing->x_res) &&
			(temp.y_res == timing->y_res)) {

			temp_hsync = temp.hfp + temp.hsw + temp.hbp;
			timing_hsync = timing->hfp + timing->hsw + timing->hbp;
			temp_vsync = temp.vfp + temp.vsw + temp.vbp;
			timing_vsync = timing->vfp + timing->vsw + timing->vbp;

			pr_info("Temp_hsync = %d , temp_vsync = %d ,\
				timing_hsync = %d, timing_vsync = %d\n",\
				temp_hsync, temp_hsync,
				timing_hsync, timing_vsync);

			if ((temp_hsync == timing_hsync)
				&&  (temp_vsync == timing_vsync)) {
				code = i;
				cm.code = code_index[i];
				if (code < 14)
					cm.mode = HDMI_HDMI;
				else
					cm.mode = HDMI_DVI;
				pr_info("Hdmi_code = %d mode = %d\n",
					 cm.code, cm.mode);
				print_omap_video_timings(&temp);
				break;
			 }
		}
	}

	return cm;
}

void get_horz_vert_timing_info(int current_descriptor_addrs, u8 *edid ,
		struct omap_video_timings *timings)
{
	/* X and Y resolution */
	timings->x_res = (((edid[current_descriptor_addrs + 4] & 0xF0) << 4) |
			 edid[current_descriptor_addrs + 2]);
	timings->y_res = (((edid[current_descriptor_addrs + 7] & 0xF0) << 4) |
			 edid[current_descriptor_addrs + 5]);

	timings->pixel_clock = ((edid[current_descriptor_addrs + 1] << 8) |
				edid[current_descriptor_addrs]);

	timings->pixel_clock = 10 * timings->pixel_clock;

	/* HORIZONTAL FRONT PORCH */
	timings->hfp = edid[current_descriptor_addrs + 8] |
			((edid[current_descriptor_addrs + 11] & 0xc0) << 2);
	/* HORIZONTAL SYNC WIDTH */
	timings->hsw = edid[current_descriptor_addrs + 9] |
			((edid[current_descriptor_addrs + 11] & 0x30) << 4);
	/* HORIZONTAL BACK PORCH */
	timings->hbp = (((edid[current_descriptor_addrs + 4] & 0x0F) << 8) |
			edid[current_descriptor_addrs + 3]) -
			(timings->hfp + timings->hsw);
	/* VERTICAL FRONT PORCH */
	timings->vfp = ((edid[current_descriptor_addrs + 10] & 0xF0) >> 4) |
			((edid[current_descriptor_addrs + 11] & 0x0f) << 2);
	/* VERTICAL SYNC WIDTH */
	timings->vsw = (edid[current_descriptor_addrs + 10] & 0x0F) |
			((edid[current_descriptor_addrs + 11] & 0x03) << 4);
	/* VERTICAL BACK PORCH */
	timings->vbp = (((edid[current_descriptor_addrs + 7] & 0x0F) << 8) |
			edid[current_descriptor_addrs + 6]) -
			(timings->vfp + timings->vsw);

	print_omap_video_timings(timings);

}

/* Description : This function gets the resolution information from EDID */
static int get_edid_timing_data(u8 *edid)
{
	u8 count, code;
	u16 current_descriptor_addrs;
	struct hdmi_cm cm;

	/* Seach block 0, there are 4 DTDs arranged in priority order */
	for (count = 0; count < EDID_SIZE_BLOCK0_TIMING_DESCRIPTOR; count++) {
		current_descriptor_addrs =
			EDID_DESCRIPTOR_BLOCK0_ADDRESS +
			count * EDID_TIMING_DESCRIPTOR_SIZE;
		get_horz_vert_timing_info(current_descriptor_addrs,
				edid, &edid_timings);
		cm = hdmi_get_code(&edid_timings);
		pr_info("Block0[%d] value matches code = %d , mode = %d\n",
			count, cm.code, cm.mode);
		if (cm.code == -1)
			continue;
		else {
			hdmi.code = cm.code;
			hdmi.mode = cm.mode;
			pr_info("code = %d , mode = %d\n",
				hdmi.code, hdmi.mode);
			return 1;
		}
	}
	if (edid[0x7e] != 0x00) {
		for (count = 0; count < EDID_SIZE_BLOCK1_TIMING_DESCRIPTOR;
			count++) {
			current_descriptor_addrs =
			EDID_DESCRIPTOR_BLOCK1_ADDRESS +
			count * EDID_TIMING_DESCRIPTOR_SIZE;
			get_horz_vert_timing_info(current_descriptor_addrs,
						edid, &edid_timings);
			cm = hdmi_get_code(&edid_timings);
			pr_info("Block1[%d] value matches code = %d , mode = %d",
				count, cm.code, cm.mode);
			if (cm.code == -1)
				continue;
			else {
				hdmi.code = cm.code;
				hdmi.mode = cm.mode;
				pr_info("code = %d , mode = %d\n",
				hdmi.code, hdmi.mode);
				return 1;
			}
		}
	}

	hdmi.code = 4; /* setting default value of 640 480 VGA */
	hdmi.mode = HDMI_DVI;
	code = code_vesa[hdmi.code];
	edid_timings = cea_vesa_timings[code].timings;

	return 1;
}

static int hdmi_read_edid(struct omap_video_timings *dp)
{
	int r = 0, ret, code;

	memset(edid, 0, HDMI_EDID_MAX_LENGTH);

	if (!edid_set)
		ret = read_edid(edid, HDMI_EDID_MAX_LENGTH);

	if (ret != 0) {
		pr_warning("HDMI failed to read E-EDID\n");
	} else {
		if (!memcmp(edid, header, sizeof(header))) {
			/* search for timings of default resolution */
			if (get_edid_timing_data(edid))
				edid_set = true;
		}
	}

	if (!edid_set) {
		pr_info("fallback to VGA\n");
		hdmi.code = 4; /* setting default value of 640 480 VGA */
		hdmi.mode = HDMI_DVI;
	}

	code = get_timings_index();

	*dp = cea_vesa_timings[code].timings;

	pr_info("hdmi read EDID\n");
	print_omap_video_timings(dp);

	return r;
}

static void hdmi_core_init(struct hdmi_core_video_config *video_cfg,
	struct hdmi_core_infoframe_avi *avi_cfg,
	struct hdmi_core_packet_enable_repeat *repeat_cfg)
{
	pr_info("Enter hdmi_core_init\n");

	/* video core */
	video_cfg->ip_bus_width = HDMI_INPUT_8BIT;
	video_cfg->op_dither_truc = HDMI_OUTPUTTRUNCATION_8BIT;
	video_cfg->deep_color_pkt = HDMI_DEEPCOLORPACKECTDISABLE;
	video_cfg->pkt_mode = HDMI_PACKETMODERESERVEDVALUE;
	video_cfg->hdmi_dvi = HDMI_DVI;
	video_cfg->tclk_sel_clkmult = FPLL10IDCK;

	/* info frame */
	avi_cfg->db1_format = 0;
	avi_cfg->db1_active_info = 0;
	avi_cfg->db1_bar_info_dv = 0;
	avi_cfg->db1_scan_info = 0;
	avi_cfg->db2_colorimetry = 0;
	avi_cfg->db2_aspect_ratio = 0;
	avi_cfg->db2_active_fmt_ar = 0;
	avi_cfg->db3_itc = 0;
	avi_cfg->db3_ec = 0;
	avi_cfg->db3_q_range = 0;
	avi_cfg->db3_nup_scaling = 0;
	avi_cfg->db4_videocode = 0;
	avi_cfg->db5_pixel_repeat = 0;
	avi_cfg->db6_7_line_eoftop = 0 ;
	avi_cfg->db8_9_line_sofbottom = 0;
	avi_cfg->db10_11_pixel_eofleft = 0;
	avi_cfg->db12_13_pixel_sofright = 0;

	/* packet enable and repeat */
	repeat_cfg->audio_pkt = 0;
	repeat_cfg->audio_pkt_repeat = 0;
	repeat_cfg->avi_infoframe = 0;
	repeat_cfg->avi_infoframe_repeat = 0;
	repeat_cfg->gen_cntrl_pkt = 0;
	repeat_cfg->gen_cntrl_pkt_repeat = 0;
	repeat_cfg->generic_pkt = 0;
	repeat_cfg->generic_pkt_repeat = 0;
}

static void hdmi_core_powerdown_disable(void)
{
	pr_info("Enter hdmi_core_powerdown_disable\n");
	REG_FLD_MOD(HDMI_CORE_SYS, HDMI_CORE_CTRL1, 0x0, 0, 0);
}

static void hdmi_core_swreset_release(void)
{
	pr_info("Enter hdmi_core_swreset_release\n");
	REG_FLD_MOD(HDMI_CORE_SYS, HDMI_CORE_SYS_SRST, 0x0, 0, 0);
}

static void hdmi_core_swreset_assert(void)
{
	pr_info("Enter hdmi_core_swreset_assert\n");
	REG_FLD_MOD(HDMI_CORE_SYS, HDMI_CORE_SYS_SRST, 0x1, 0, 0);
}

/* DSS_HDMI_CORE_VIDEO_CONFIG */
static int hdmi_core_video_config(
	struct hdmi_core_video_config *cfg)
{
	u32 name = HDMI_CORE_SYS;
	u32 av_name = HDMI_CORE_AV;
	u32 r = 0;

	/* sys_ctrl1 default configuration not tunable */
	u32 ven;
	u32 hen;
	u32 bsel;
	u32 edge;

	/* sys_ctrl1 default configuration not tunable */
	ven = HDMI_CORE_CTRL1_VEN_FOLLOWVSYNC;
	hen = HDMI_CORE_CTRL1_HEN_FOLLOWHSYNC;
	bsel = HDMI_CORE_CTRL1_BSEL_24BITBUS;
	edge = HDMI_CORE_CTRL1_EDGE_RISINGEDGE;

	/* sys_ctrl1 default configuration not tunable */
	r = hdmi_read_reg(name, HDMI_CORE_CTRL1);
	r = FLD_MOD(r, ven, 5, 5);
	r = FLD_MOD(r, hen, 4, 4);
	r = FLD_MOD(r, bsel, 2, 2);
	r = FLD_MOD(r, edge, 1, 1);
	hdmi_write_reg(name, HDMI_CORE_CTRL1, r);

	REG_FLD_MOD(name, HDMI_CORE_SYS_VID_ACEN, cfg->ip_bus_width, 7, 6);

	/* Vid_Mode */
	r = hdmi_read_reg(name, HDMI_CORE_SYS_VID_MODE);

	/* dither truncation configuration */
	if (cfg->op_dither_truc >
				HDMI_OUTPUTTRUNCATION_12BIT) {
		r = FLD_MOD(r, cfg->op_dither_truc - 3, 7, 6);
		r = FLD_MOD(r, 1, 5, 5);
	} else {
		r = FLD_MOD(r, cfg->op_dither_truc, 7, 6);
		r = FLD_MOD(r, 0, 5, 5);
	}
	hdmi_write_reg(name, HDMI_CORE_SYS_VID_MODE, r);

	/* HDMI_Ctrl */
	r = hdmi_read_reg(av_name, HDMI_CORE_AV_HDMI_CTRL);
	r = FLD_MOD(r, cfg->deep_color_pkt, 6, 6);
	r = FLD_MOD(r, cfg->pkt_mode, 5, 3);
	r = FLD_MOD(r, cfg->hdmi_dvi, 0, 0);
	hdmi_write_reg(av_name, HDMI_CORE_AV_HDMI_CTRL, r);

	/* TMDS_CTRL */
	REG_FLD_MOD(name, HDMI_CORE_SYS_TMDS_CTRL,
		cfg->tclk_sel_clkmult, 6, 5);

	return 0;
}

static int hdmi_core_aux_infoframe_avi_config(
	struct hdmi_core_infoframe_avi info_avi)
{
	u16 offset;
	int dbyte, dbyte_size;
	u32 val;
	char sum = 0, checksum = 0;

	dbyte = HDMI_CORE_AV_AVI_DBYTE;
	dbyte_size = HDMI_CORE_AV_AVI_DBYTE_ELSIZE;

	sum += 0x82 + 0x002 + 0x00D;
	hdmi_write_reg(HDMI_CORE_AV, HDMI_CORE_AV_AVI_TYPE, 0x082);
	hdmi_write_reg(HDMI_CORE_AV, HDMI_CORE_AV_AVI_VERS, 0x002);
	hdmi_write_reg(HDMI_CORE_AV, HDMI_CORE_AV_AVI_LEN, 0x00D);

	offset = dbyte + (0 * dbyte_size);
	val = (info_avi.db1_format << 5) |
		(info_avi.db1_active_info << 4) |
		(info_avi.db1_bar_info_dv << 2) |
		(info_avi.db1_scan_info);
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (1 * dbyte_size);
	val = (info_avi.db2_colorimetry << 6) |
		(info_avi.db2_aspect_ratio << 4) |
		(info_avi.db2_active_fmt_ar);
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (2 * dbyte_size);
	val = (info_avi.db3_itc << 7) |
		(info_avi.db3_ec << 4) |
		(info_avi.db3_q_range << 2) |
		(info_avi.db3_nup_scaling);
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (3 * dbyte_size);
	hdmi_write_reg(HDMI_CORE_AV, offset, info_avi.db4_videocode);
	sum += info_avi.db4_videocode;

	offset = dbyte + (4 * dbyte_size);
	val = info_avi.db5_pixel_repeat;
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (5 * dbyte_size);
	val = info_avi.db6_7_line_eoftop & 0x00FF;
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (6 * dbyte_size);
	val = ((info_avi.db6_7_line_eoftop >> 8) & 0x00FF);
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (7 * dbyte_size);
	val = info_avi.db8_9_line_sofbottom & 0x00FF;
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (8 * dbyte_size);
	val = ((info_avi.db8_9_line_sofbottom >> 8) & 0x00FF);
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (9 * dbyte_size);
	val = info_avi.db10_11_pixel_eofleft & 0x00FF;
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (10 * dbyte_size);
	val = ((info_avi.db10_11_pixel_eofleft >> 8) & 0x00FF);
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	offset = dbyte + (11 * dbyte_size);
	val = info_avi.db12_13_pixel_sofright & 0x00FF;
	hdmi_write_reg(HDMI_CORE_AV, offset , val);
	sum += val;

	offset = dbyte + (12 * dbyte_size);
	val = ((info_avi.db12_13_pixel_sofright >> 8) & 0x00FF);
	hdmi_write_reg(HDMI_CORE_AV, offset, val);
	sum += val;

	checksum = 0x100 - sum;
	hdmi_write_reg(HDMI_CORE_AV, HDMI_CORE_AV_AVI_CHSUM, checksum);

	return 0;
}

static int hdmi_core_av_packet_config(u32 name,
	struct hdmi_core_packet_enable_repeat repeat_cfg)
{
	/* enable/repeat the infoframe */
	hdmi_write_reg(name, HDMI_CORE_AV_PB_CTRL1,
		(repeat_cfg.audio_pkt << 5)|
		(repeat_cfg.audio_pkt_repeat << 4)|
		(repeat_cfg.avi_infoframe << 1)|
		(repeat_cfg.avi_infoframe_repeat));

	/* enable/repeat the packet */
	hdmi_write_reg(name, HDMI_CORE_AV_PB_CTRL2,
		(repeat_cfg.gen_cntrl_pkt << 3)|
		(repeat_cfg.gen_cntrl_pkt_repeat << 2)|
		(repeat_cfg.generic_pkt << 1)|
		(repeat_cfg.generic_pkt_repeat));
	return 0;
}

static void hdmi_wp_init(struct hdmi_video_timing *timings,
	struct hdmi_video_format *video_fmt,
	struct hdmi_video_interface *video_int,
	struct hdmi_irq_vector *irq_enable)
{
	pr_info("Enter hdmi_wp_init\n");

	timings->hbp = 0;
	timings->hfp = 0;
	timings->hsw = 0;
	timings->vbp = 0;
	timings->vfp = 0;
	timings->vsw = 0;

	video_fmt->packing_mode = HDMI_PACK_10b_RGB_YUV444;
	video_fmt->y_res = 0;
	video_fmt->x_res = 0;

	video_int->vsp = 0;
	video_int->hsp = 0;

	video_int->interlacing = 0;
	video_int->tm = 0; /* HDMI_TIMING_SLAVE */

	irq_enable->pll_recal = 0;
	irq_enable->pll_unlock = 0;
	irq_enable->pll_lock = 0;
	irq_enable->phy_disconnect = 1;
	irq_enable->phy_connect = 1;
	irq_enable->phy_short_5v = 0;
	irq_enable->video_end_fr = 0;
	irq_enable->video_vsync = 0;
	irq_enable->fifo_sample_req = 0;
	irq_enable->fifo_overflow = 0;
	irq_enable->fifo_underflow = 0;
	irq_enable->ocp_timeout = 0;
	irq_enable->core = 1;
}

static void hdmi_wp_irq_enable(struct hdmi_irq_vector *irq_enable)
{
	u32 r = 0;

	r = ((irq_enable->pll_recal << 31) |
		(irq_enable->pll_unlock << 30) |
		(irq_enable->pll_lock << 29) |
		(irq_enable->phy_disconnect << 26) |
		(irq_enable->phy_connect << 25) |
		(irq_enable->phy_short_5v << 24) |
		(irq_enable->video_end_fr << 17) |
		(irq_enable->video_vsync << 16) |
		(irq_enable->fifo_sample_req << 10) |
		(irq_enable->fifo_overflow << 9) |
		(irq_enable->fifo_underflow << 8) |
		(irq_enable->ocp_timeout << 4) |
		(irq_enable->core << 0));

	hdmi_write_reg(HDMI_WP, HDMI_WP_IRQENABLE_SET, r);
}

/* PHY_PWR_CMD */
int hdmi_wait_phy_pwr(int val)
{
	REG_FLD_MOD(HDMI_WP, HDMI_WP_PWR_CTRL, val, 7, 6);

	if (hdmi_wait_for_bit_change(HDMI_WP,
		HDMI_WP_PWR_CTRL, 5, 4, val) != val) {
		pr_err("Failed to set PHY power mode to %d\n", val);
		return -ENODEV;
	}

	return 0;
}

/* PLL_PWR_CMD */
int hdmi_wait_pll_pwr(int val)
{
	REG_FLD_MOD(HDMI_WP, HDMI_WP_PWR_CTRL, val, 3, 2);

	/* wait till PHY_PWR_STATUS=ON */
	if (hdmi_wait_for_bit_change(HDMI_WP,
		HDMI_WP_PWR_CTRL, 1, 0, val) != val) {
		pr_err("Failed to set PHY_PWR_STATUS to ON\n");
		return -ENODEV;
	}

	return 0;
}

void hdmi_wp_video_stop(void)
{
	REG_FLD_MOD(HDMI_WP, HDMI_WP_VIDEO_CFG, 0, 31, 31);
}

void hdmi_wp_video_start(void)
{
	REG_FLD_MOD(HDMI_WP, HDMI_WP_VIDEO_CFG, (u32)0x1, 31, 31);
}

static void hdmi_wp_video_init_format(struct hdmi_video_format *video_fmt,
	struct hdmi_video_timing *timings, struct hdmi_config *param)
{
	pr_info("Enter hdmi_wp_video_init_format\n");

	video_fmt->y_res = param->lpp;
	video_fmt->x_res = param->ppl;

	timings->hbp = param->hbp;
	timings->hfp = param->hfp;
	timings->hsw = param->hsw;
	timings->vbp = param->vbp;
	timings->vfp = param->vfp;
	timings->vsw = param->vsw;
}

static void hdmi_wp_video_config_format(
	struct hdmi_video_format *video_fmt)
{
	u32 l = 0;

	REG_FLD_MOD(HDMI_WP, HDMI_WP_VIDEO_CFG, video_fmt->packing_mode, 10, 8);

	l |= FLD_VAL(video_fmt->y_res, 31, 16);
	l |= FLD_VAL(video_fmt->x_res, 15, 0);
	hdmi_write_reg(HDMI_WP, HDMI_WP_VIDEO_SIZE, l);
}

static void hdmi_wp_video_config_interface(
	struct hdmi_video_interface *video_int)
{
	u32 r;
	pr_info("Enter hdmi_wp_video_config_interface\n");

	r = hdmi_read_reg(HDMI_WP, HDMI_WP_VIDEO_CFG);
	r = FLD_MOD(r, video_int->vsp, 7, 7);
	r = FLD_MOD(r, video_int->hsp, 6, 6);
	r = FLD_MOD(r, video_int->interlacing, 3, 3);
	r = FLD_MOD(r, video_int->tm, 1, 0);
	hdmi_write_reg(HDMI_WP, HDMI_WP_VIDEO_CFG, r);
}

static void hdmi_wp_video_config_timing(
	struct hdmi_video_timing *timings)
{
	u32 timing_h = 0;
	u32 timing_v = 0;

	pr_info("Enter hdmi_wp_video_config_timing\n");

	timing_h |= FLD_VAL(timings->hbp, 31, 20);
	timing_h |= FLD_VAL(timings->hfp, 19, 8);
	timing_h |= FLD_VAL(timings->hsw, 7, 0);
	hdmi_write_reg(HDMI_WP, HDMI_WP_VIDEO_TIMING_H, timing_h);

	timing_v |= FLD_VAL(timings->vbp, 31, 20);
	timing_v |= FLD_VAL(timings->vfp, 19, 8);
	timing_v |= FLD_VAL(timings->vsw, 7, 0);
	hdmi_write_reg(HDMI_WP, HDMI_WP_VIDEO_TIMING_V, timing_v);
}

int hdmi_lib_enable(struct hdmi_config *cfg)
{
	u32 av_name = HDMI_CORE_AV;

	/* HDMI */
	struct hdmi_video_timing video_timing;
	struct hdmi_video_format video_format;
	struct hdmi_video_interface video_interface;
	struct hdmi_irq_vector irq_enable;
	/* HDMI core */
	struct hdmi_core_infoframe_avi avi_cfg;
	struct hdmi_core_video_config v_core_cfg;
	struct hdmi_core_packet_enable_repeat repeat_cfg;

	hdmi_wp_init(&video_timing, &video_format,
		&video_interface, &irq_enable);

	hdmi_core_init(&v_core_cfg,
		&avi_cfg,
		&repeat_cfg);

	/* Enable PLL Lock and UnLock intrerrupts */
	irq_enable.pll_unlock = 1;
	irq_enable.pll_lock = 1;
	irq_enable.core = 1;

	/* init DSS register */
	hdmi_wp_irq_enable(&irq_enable);

	hdmi_wp_video_init_format(&video_format,
			&video_timing, cfg);

	hdmi_wp_video_config_timing(&video_timing);

	/* video config */
	video_format.packing_mode = HDMI_PACK_24b_RGB_YUV444_YUV422;

	hdmi_wp_video_config_format(&video_format);

	video_interface.vsp = cfg->v_pol;
	video_interface.hsp = cfg->h_pol;
	video_interface.interlacing = cfg->interlace;
	video_interface.tm = 1 ; /* HDMI_TIMING_MASTER_24BIT */

	hdmi_wp_video_config_interface(&video_interface);

	/*
	 * configure core video part
	 * set software reset in the core
	 */
	hdmi_core_swreset_assert();

	/* power down off */
	hdmi_core_powerdown_disable();

	v_core_cfg.pkt_mode = HDMI_PACKETMODE24BITPERPIXEL;
	v_core_cfg.hdmi_dvi = cfg->hdmi_dvi;

	hdmi_core_video_config(&v_core_cfg);

	/* release software reset in the core */
	hdmi_core_swreset_release();

	/*
	 * configure packet
	 * info frame video see doc CEA861-D page 65
	 */
	avi_cfg.db1_format = INFOFRAME_AVI_DB1Y_RGB;
	avi_cfg.db1_active_info =
		INFOFRAME_AVI_DB1A_ACTIVE_FORMAT_OFF;
	avi_cfg.db1_bar_info_dv = INFOFRAME_AVI_DB1B_NO;
	avi_cfg.db1_scan_info = INFOFRAME_AVI_DB1S_0;
	avi_cfg.db2_colorimetry = INFOFRAME_AVI_DB2C_NO;
	avi_cfg.db2_aspect_ratio = INFOFRAME_AVI_DB2M_NO;
	avi_cfg.db2_active_fmt_ar = INFOFRAME_AVI_DB2R_SAME;
	avi_cfg.db3_itc = INFOFRAME_AVI_DB3ITC_NO;
	avi_cfg.db3_ec = INFOFRAME_AVI_DB3EC_XVYUV601;
	avi_cfg.db3_q_range = INFOFRAME_AVI_DB3Q_DEFAULT;
	avi_cfg.db3_nup_scaling = INFOFRAME_AVI_DB3SC_NO;
	avi_cfg.db4_videocode = cfg->video_format;
	avi_cfg.db5_pixel_repeat = INFOFRAME_AVI_DB5PR_NO;
	avi_cfg.db6_7_line_eoftop = 0;
	avi_cfg.db8_9_line_sofbottom = 0;
	avi_cfg.db10_11_pixel_eofleft = 0;
	avi_cfg.db12_13_pixel_sofright = 0;

	hdmi_core_aux_infoframe_avi_config(avi_cfg);

	/* enable/repeat the infoframe */
	repeat_cfg.avi_infoframe = PACKETENABLE;
	repeat_cfg.avi_infoframe_repeat = PACKETREPEATON;

	/* wakeup */
	repeat_cfg.audio_pkt = PACKETENABLE;
	repeat_cfg.audio_pkt_repeat = PACKETREPEATON;
	hdmi_core_av_packet_config(av_name, repeat_cfg);

	return 0;
}

int hdmi_lib_init(void)
{
	/* Base address can be taken from Platform */
	hdmi.base_wp = ioremap(HDMI_WP_BASE, (HDMI_HDCP_BASE - HDMI_WP_BASE));

	if (!hdmi.base_wp) {
		pr_err("can't ioremap WP\n");
		return -ENOMEM;
	}

	return 0;
}

void hdmi_lib_exit(void)
{
	iounmap(hdmi.base_wp);
}

int hdmi_wait_softreset(void)
{
	/* reset W1 */
	REG_FLD_MOD(HDMI_WP, HDMI_WP_SYSCONFIG, 0x1, 0, 0);

	/* wait till SOFTRESET == 0 */
	if (hdmi_wait_for_bit_change(HDMI_WP,
		HDMI_WP_SYSCONFIG, 0, 0, 0) != 0) {
		pr_err("sysconfig reset failed\n");
		return -ENODEV;
	}

	return 0;
}

static void update_hdmi_timings(struct hdmi_config *cfg,
	struct omap_video_timings *timings, int code)
{
	cfg->ppl = timings->x_res;
	cfg->lpp = timings->y_res;
	cfg->hbp = timings->hbp;
	cfg->hfp = timings->hfp;
	cfg->hsw = timings->hsw;
	cfg->vbp = timings->vbp;
	cfg->vfp = timings->vfp;
	cfg->vsw = timings->vsw;
	cfg->pixel_clock = timings->pixel_clock;
	cfg->v_pol = cea_vesa_timings[code].vsync_pol;
	cfg->h_pol = cea_vesa_timings[code].hsync_pol;
}

static int hdmi_panel_probe(struct omap_dss_device *dssdev)
{
	int code;
	pr_info("ENTER hdmi_panel_probe\n");

	dssdev->panel.config = OMAP_DSS_LCD_TFT |
			OMAP_DSS_LCD_IVS | OMAP_DSS_LCD_IHS;

	code = get_timings_index();

	dssdev->panel.timings = cea_vesa_timings[code].timings;

	pr_info("hdmi_panel_probe x_res= %d y_res = %d\n",
		dssdev->panel.timings.x_res,
		dssdev->panel.timings.y_res);

	mdelay(50);

	return 0;
}

static void hdmi_panel_remove(struct omap_dss_device *dssdev)
{

}

static int hdmi_panel_enable(struct omap_dss_device *dssdev)
{
	int r, code = 0;
	struct hdmi_pll_info pll_data;
	struct omap_video_timings *p;
	int clkin, n, phy;

	/* the tv overlay manager is shared */
	if (dssdev->state != OMAP_DSS_DISPLAY_DISABLED)
		return -EINVAL;

	hdmi_wp_video_stop();

	p = &dssdev->panel.timings;

	pr_info("hdmi_panel_enable x_res= %d y_res = %d\n",
		dssdev->panel.timings.x_res,
		dssdev->panel.timings.y_res);

	r = omapdss_hdmi_display_enable(dssdev);

	if (r) {
		pr_info("failed to power on\n");
		return r;
	}

	pr_info("No edid set thus will be calling hdmi_read_edid\n");
	r = hdmi_read_edid(p);
	if (r)
		return -EIO;

	code = get_timings_index();
	dssdev->panel.timings = cea_vesa_timings[code].timings;
	update_hdmi_timings(&hdmi.cfg, p, code);

	clkin = 3840; /* 38.4 mHz */
	n = 15; /* this is a constant for our math */
	phy = p->pixel_clock;

	omapdss_hdmi_compute_pll(clkin, phy, n, &pll_data);

	/* config the PLL and PHY first */
	r = hdmi_pll_program(&pll_data);
	if (r) {
		pr_info("Failed to lock PLL\n");
		return -EIO;
	}

	r = hdmi_phy_init(HDMI_WP, HDMI_PHY);
	if (r) {
		pr_info("Failed to start PHY\n");
		return -EIO;
	}

	hdmi.cfg.hdmi_dvi = hdmi.mode;
	hdmi.cfg.video_format = hdmi.code;
	hdmi_lib_enable(&hdmi.cfg);

	omapdss_hdmi_dispc_setting(dssdev);

	hdmi_wp_video_start();

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	return 0;
}

static void hdmi_panel_disable(struct omap_dss_device *dssdev)
{
	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE) {
		hdmi_wp_video_stop();
		hdmi_phy_off();
		hdmi_wait_pll_pwr(HDMI_PLLPWRCMD_ALLOFF);
		omapdss_hdmi_display_disable(dssdev);
		edid_set = 0;
	}

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
}

static int hdmi_panel_suspend(struct omap_dss_device *dssdev)
{
	if (dssdev->state == OMAP_DSS_DISPLAY_DISABLED ||
		dssdev->state == OMAP_DSS_DISPLAY_SUSPENDED)
		return -EINVAL;

	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;

	omapdss_hdmi_display_suspend(dssdev);

	return 0;
}

static int hdmi_panel_resume(struct omap_dss_device *dssdev)
{
	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE)
		return -EINVAL;

	omapdss_hdmi_display_resume(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	return 0;
}

static void hdmi_get_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	*timings = dssdev->panel.timings;
}

static void hdmi_set_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	pr_info("hdmi_set_timings\n");

	dssdev->panel.timings = *timings;

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE) {
		/* turn the hdmi off and on to get new timings to use */
		omapdss_hdmi_display_disable(dssdev);
		omapdss_hdmi_display_enable(dssdev);
	}
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
	hdmi_lib_init();
	omap_dss_register_driver(&hdmi_driver);
	return 0;
}

static void __exit hdmi_panel_exit(void)
{
	hdmi_lib_exit();
	omap_dss_unregister_driver(&hdmi_driver);

}

module_init(hdmi_panel_init);
module_exit(hdmi_panel_exit);

