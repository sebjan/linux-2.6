/*
 * hdmi_omap4_panel.h
 *
 * HDMI driver definition for TI OMAP4 processors.
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com/
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
 * History:
 */

#ifndef _HDMI_OMAP4_PANEL_H_
#define _HDMI_OMAP4_PANEL_H_

#include <linux/string.h>
#include <plat/display.h>

#define HDMI_WP			0x0
#define HDMI_CORE_SYS		0x400
#define HDMI_CORE_AV		0x900
#define HDMI_PLLCTRL		0x200
#define HDMI_PHY		0x300



/* HDMI PHY */
#define HDMI_TXPHY_TX_CTRL				0x0
#define HDMI_TXPHY_DIGITAL_CTRL				0x4
#define HDMI_TXPHY_POWER_CTRL				0x8

/* HDMI Wrapper */
#define HDMI_WP_REVISION				0x0
#define HDMI_WP_SYSCONFIG				0x10
#define HDMI_WP_IRQSTATUS_RAW				0x24
#define HDMI_WP_IRQSTATUS				0x28
#define HDMI_WP_PWR_CTRL				0x40
#define HDMI_WP_IRQENABLE_SET				0x2C
#define HDMI_WP_VIDEO_CFG				0x50
#define HDMI_WP_VIDEO_SIZE				0x60
#define HDMI_WP_VIDEO_TIMING_H				0x68
#define HDMI_WP_VIDEO_TIMING_V				0x6C
#define HDMI_WP_WP_CLK					0x70

/* HDMI IP Core System */
#define HDMI_CORE_SYS_VND_IDL				0x0
#define HDMI_CORE_SYS_DEV_IDL				0x8
#define HDMI_CORE_SYS_DEV_IDH				0xC
#define HDMI_CORE_SYS_DEV_REV				0x10
#define HDMI_CORE_SYS_SRST				0x14
#define HDMI_CORE_CTRL1					0x20
#define HDMI_CORE_SYS_SYS_STAT				0x24
#define HDMI_CORE_SYS_VID_ACEN				0x124
#define HDMI_CORE_SYS_VID_MODE				0x128
#define HDMI_CORE_SYS_INTR_STATE			0x1C0
#define HDMI_CORE_SYS_INTR1				0x1C4
#define HDMI_CORE_SYS_INTR2				0x1C8
#define HDMI_CORE_SYS_INTR3				0x1CC
#define HDMI_CORE_SYS_INTR4				0x1D0
#define HDMI_CORE_SYS_UMASK1				0x1D4
#define HDMI_CORE_SYS_TMDS_CTRL				0x208
#define HDMI_CORE_CTRL1_VEN_FOLLOWVSYNC			0x1
#define HDMI_CORE_CTRL1_HEN_FOLLOWHSYNC			0x1
#define HDMI_CORE_CTRL1_BSEL_24BITBUS			0x1
#define HDMI_CORE_CTRL1_EDGE_RISINGEDGE			0x1
#define HDMI_CORE_SYS_DE_DLY				0xC8
#define HDMI_CORE_SYS_DE_CTRL				0xCC
#define HDMI_CORE_SYS_DE_TOP				0xD0
#define HDMI_CORE_SYS_DE_CNTL				0xD8
#define HDMI_CORE_SYS_DE_CNTH				0xDC
#define HDMI_CORE_SYS_DE_LINL				0xE0
#define HDMI_CORE_SYS_DE_LINH_1				0xE4

/* HDMI IP Core Audio Video */
#define HDMI_CORE_AV_HDMI_CTRL				0xBC
#define HDMI_CORE_AV_DPD				0xF4
#define HDMI_CORE_AV_PB_CTRL1				0xF8
#define HDMI_CORE_AV_PB_CTRL2				0xFC
#define HDMI_CORE_AV_AVI_TYPE				0x100
#define HDMI_CORE_AV_AVI_VERS				0x104
#define HDMI_CORE_AV_AVI_LEN				0x108
#define HDMI_CORE_AV_AVI_CHSUM				0x10C
#define HDMI_CORE_AV_AVI_DBYTE				0x110
#define HDMI_CORE_AV_AVI_DBYTE_ELSIZE			0x4

/* HDMI DDC E-DID */
#define HDMI_CORE_DDC_CMD				0x3CC
#define HDMI_CORE_DDC_STATUS				0x3C8
#define HDMI_CORE_DDC_ADDR				0x3B4
#define HDMI_CORE_DDC_OFFSET				0x3BC
#define HDMI_CORE_DDC_COUNT1				0x3C0
#define HDMI_CORE_DDC_COUNT2				0x3C4
#define HDMI_CORE_DDC_DATA				0x3D0
#define HDMI_CORE_DDC_SEGM				0x3B8

#define HDMI_CORE_AV_AVI_DBYTE				0x110
#define HDMI_CORE_AV_AVI_DBYTE_ELSIZE			0x4
#define HDMI_IP_CORE_AV_AVI_DBYTE_NELEMS		15
#define HDMI_CORE_AV_SPD_DBYTE				0x190
#define HDMI_CORE_AV_SPD_DBYTE_ELSIZE			0x4
#define HDMI_CORE_AV_SPD_DBYTE_NELEMS			27
#define HDMI_CORE_AV_MPEG_DBYTE				0x290
#define HDMI_CORE_AV_MPEG_DBYTE_ELSIZE			0x4
#define HDMI_CORE_AV_MPEG_DBYTE_NELEMS			27
#define HDMI_CORE_AV_GEN_DBYTE				0x300
#define HDMI_CORE_AV_GEN_DBYTE_ELSIZE			0x4
#define HDMI_CORE_AV_GEN_DBYTE_NELEMS			31
#define HDMI_CORE_AV_GEN2_DBYTE				0x380
#define HDMI_CORE_AV_GEN2_DBYTE_ELSIZE			0x4
#define HDMI_CORE_AV_GEN2_DBYTE_NELEMS			31
#define HDMI_CORE_AV_ACR_CTRL				0x4
#define HDMI_CORE_AV_FREQ_SVAL				0x8
#define HDMI_CORE_AV_N_SVAL1				0xC
#define HDMI_CORE_AV_N_SVAL2				0x10
#define HDMI_CORE_AV_N_SVAL3				0x14
#define HDMI_CORE_AV_CTS_SVAL1				0x18
#define HDMI_CORE_AV_CTS_SVAL2				0x1C
#define HDMI_CORE_AV_CTS_SVAL3				0x20
#define HDMI_CORE_AV_CTS_HVAL1				0x24
#define HDMI_CORE_AV_CTS_HVAL2				0x28
#define HDMI_CORE_AV_CTS_HVAL3				0x2C
#define HDMI_CORE_AV_AUD_MODE				0x50
#define HDMI_CORE_AV_SPDIF_CTRL				0x54
#define HDMI_CORE_AV_HW_SPDIF_FS			0x60
#define HDMI_CORE_AV_SWAP_I2S				0x64
#define HDMI_CORE_AV_SPDIF_ERTH				0x6C
#define HDMI_CORE_AV_I2S_IN_MAP				0x70
#define HDMI_CORE_AV_I2S_IN_CTRL			0x74
#define HDMI_CORE_AV_I2S_CHST0				0x78
#define HDMI_CORE_AV_I2S_CHST1				0x7C
#define HDMI_CORE_AV_I2S_CHST2				0x80
#define HDMI_CORE_AV_I2S_CHST4				0x84
#define HDMI_CORE_AV_I2S_CHST5				0x88
#define HDMI_CORE_AV_ASRC				0x8C
#define HDMI_CORE_AV_I2S_IN_LEN				0x90
#define HDMI_CORE_AV_HDMI_CTRL				0xBC
#define HDMI_CORE_AV_AUDO_TXSTAT			0xC0
#define HDMI_CORE_AV_AUD_PAR_BUSCLK_1			0xCC
#define HDMI_CORE_AV_AUD_PAR_BUSCLK_2			0xD0
#define HDMI_CORE_AV_AUD_PAR_BUSCLK_3			0xD4
#define HDMI_CORE_AV_TEST_TXCTRL			0xF0
#define HDMI_CORE_AV_DPD				0xF4
#define HDMI_CORE_AV_PB_CTRL1				0xF8
#define HDMI_CORE_AV_PB_CTRL2				0xFC
#define HDMI_CORE_AV_AVI_TYPE				0x100
#define HDMI_CORE_AV_AVI_VERS				0x104
#define HDMI_CORE_AV_AVI_LEN				0x108
#define HDMI_CORE_AV_AVI_CHSUM				0x10C
#define HDMI_CORE_AV_SPD_TYPE				0x180
#define HDMI_CORE_AV_SPD_VERS				0x184
#define HDMI_CORE_AV_SPD_LEN				0x188
#define HDMI_CORE_AV_SPD_CHSUM				0x18C
#define HDMI_CORE_AV_MPEG_TYPE				0x280
#define HDMI_CORE_AV_MPEG_VERS				0x284
#define HDMI_CORE_AV_MPEG_LEN				0x288
#define HDMI_CORE_AV_MPEG_CHSUM				0x28C
#define HDMI_CORE_AV_CP_BYTE1				0x37C
#define HDMI_CORE_AV_CEC_ADDR_ID			0x3FC

/* PLL */
#define PLLCTRL_PLL_CONTROL				0x0
#define PLLCTRL_PLL_STATUS				0x4
#define PLLCTRL_PLL_GO					0x8
#define PLLCTRL_CFG1					0xC
#define PLLCTRL_CFG2					0x10
#define PLLCTRL_CFG3					0x14
#define PLLCTRL_CFG4					0x20

/* HDMI PHY */
#define HDMI_TXPHY_TX_CTRL				0x0
#define HDMI_TXPHY_DIGITAL_CTRL				0x4
#define HDMI_TXPHY_POWER_CTRL				0x8
#define HDMI_TXPHY_PAD_CFG_CTRL				0xC

/* HDMI EDID Length  */
#define HDMI_EDID_MAX_LENGTH				256
#define EDID_TIMING_DESCRIPTOR_SIZE			0x12
#define EDID_DESCRIPTOR_BLOCK0_ADDRESS			0x36
#define EDID_DESCRIPTOR_BLOCK1_ADDRESS			0x80
#define EDID_SIZE_BLOCK0_TIMING_DESCRIPTOR		4
#define EDID_SIZE_BLOCK1_TIMING_DESCRIPTOR		4

#define OMAP_HDMI_TIMINGS_NB				34

#define FLD_MASK(start, end)	(((1 << (start - end + 1)) - 1) << (end))
#define FLD_VAL(val, start, end) (((val) << end) & FLD_MASK(start, end))
#define FLD_GET(val, start, end) (((val) & FLD_MASK(start, end)) >> (end))
#define FLD_MOD(orig, val, start, end) \
	(((orig) & ~FLD_MASK(start, end)) | FLD_VAL(val, start, end))

#define REG_FLD_MOD(base, idx, val, start, end) \
	hdmi_write_reg(base, idx, FLD_MOD(hdmi_read_reg(base, idx), \
			val, start, end))

#define RD_REG_32(COMP, REG)            hdmi_read_reg(COMP, REG)
#define WR_REG_32(COMP, REG, VAL)       hdmi_write_reg(COMP, REG, (u32)(VAL))


u8		edid[HDMI_EDID_MAX_LENGTH] = {0};
u8		edid_set;
u8		header[8] = {0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0};
struct		omap_video_timings edid_timings;

/* HDMI timing structure */
struct hdmi_timings {
	struct omap_video_timings timings;
	int vsync_pol;
	int hsync_pol;
};

/*
 * Logic for the below structure
 * user enters the CEA or VESA timings by specifying
 * the hdmicode which corresponds to CEA/VESA timings
 * please refer to section 6.3 in HDMI 1.3 specification for timing code.
 * There is a correspondence between CEA/VESA timing and code.
 * In the below structure, cea_vesa_timings corresponds to all
 * The OMAP4 supported timing  CEA and VESA timing values.
 * code_cea corresponds to the CEA code entered by the user,
 * The use of it is to get the timing from the cea_vesa_timing array.
 * Similarly for code_vesa.
 * code_index is backmapping, Once EDID is read from the TV
 * EDID is parsed to find the timing values to map it back to the
 * corresponding CEA or VESA index this structure is used.
 */

/*
 * This is the structure which has all supported timing
 * values that OMAP4 supports
 */
struct hdmi_timings cea_vesa_timings[OMAP_HDMI_TIMINGS_NB] = {
	{ {640, 480, 25200, 96, 16, 48, 2, 10, 33} , 0 , 0},
	{ {1280, 720, 74250, 40, 440, 220, 5, 5, 20}, 1, 1},
	{ {1280, 720, 74250, 40, 110, 220, 5, 5, 20}, 1, 1},
	{ {720, 480, 27027, 62, 16, 60, 6, 9, 30}, 0, 0},
	{ {2880, 576, 108000, 256, 48, 272, 5, 5, 39}, 0, 0},
	{ {1440, 240, 27027, 124, 38, 114, 3, 4, 15}, 0, 0},
	{ {1440, 288, 27000, 126, 24, 138, 3, 2, 19}, 0, 0},
	{ {1920, 540, 74250, 44, 528, 148, 5, 2, 15}, 1, 1},
	{ {1920, 540, 74250, 44, 88, 148, 5, 2, 15}, 1, 1},
	{ {1920, 1080, 148500, 44, 88, 148, 5, 4, 36}, 1, 1},
	{ {720, 576, 27000, 64, 12, 68, 5, 5, 39}, 0, 0},
	{ {1440, 576, 54000, 128, 24, 136, 5, 5, 39}, 0, 0},
	{ {1920, 1080, 148500, 44, 528, 148, 5, 4, 36}, 1, 1},
	{ {2880, 480, 108108, 248, 64, 240, 6, 9, 30}, 0, 0},
	{ {1920, 1080, 74250, 44, 638, 148, 5, 4, 36}, 1, 1},
	/* VESA From Here */
	{ {640, 480, 25175, 96, 16, 48, 2 , 11, 31}, 0, 0},
	{ {800, 600, 40000, 128, 40, 88, 4 , 1, 23}, 1, 1},
	{ {848, 480, 33750, 112, 16, 112, 8 , 6, 23}, 1, 1},
	{ {1280, 768, 79500, 128, 64, 192, 7 , 3, 20}, 1, 0},
	{ {1280, 800, 83500, 128, 72, 200, 6 , 3, 22}, 1, 0},
	{ {1360, 768, 85500, 112, 64, 256, 6 , 3, 18}, 1, 1},
	{ {1280, 960, 108000, 112, 96, 312, 3 , 1, 36}, 1, 1},
	{ {1280, 1024, 108000, 112, 48, 248, 3 , 1, 38}, 1, 1},
	{ {1024, 768, 65000, 136, 24, 160, 6, 3, 29}, 0, 0},
	{ {1400, 1050, 121750, 144, 88, 232, 4, 3, 32}, 1, 0},
	{ {1440, 900, 106500, 152, 80, 232, 6, 3, 25}, 1, 0},
	{ {1680, 1050, 146250, 176 , 104, 280, 6, 3, 30}, 1, 0},
	{ {1366, 768, 85500, 143, 70, 213, 3, 3, 24}, 1, 1},
	{ {1920, 1080, 148500, 44, 88, 80, 5, 4, 36}, 1, 1},
	{ {1280, 768, 68250, 32, 48, 80, 7, 3, 12}, 0, 1},
	{ {1400, 1050, 101000, 32, 48, 80, 4, 3, 23}, 0, 1},
	{ {1680, 1050, 119000, 32, 48, 80, 6, 3, 21}, 0, 1},
	{ {1280, 800, 79500, 32, 48, 80, 6, 3, 14}, 0, 1},
	{ {1280, 720, 74250, 40, 110, 220, 5, 5, 20}, 1, 1}
};

/*
 * This is a static mapping array which maps the timing values
 * with corresponding CEA / VESA code
 */
static int code_index[OMAP_HDMI_TIMINGS_NB] = {
	1, 19, 4, 2, 37, 6, 21, 20, 5, 16, 17, 29, 31, 35, 32,
	/* <--15 CEA 17--> vesa*/
	4, 9, 0xE, 0x17, 0x1C, 0x27, 0x20, 0x23, 0x10, 0x2A,
	0X2F, 0x3A, 0X51, 0X52, 0x16, 0x29, 0x39, 0x1B
};

/*
 * This is reverse static mapping which maps the CEA / VESA code
 * to the corresponding timing values
 */
static int code_cea[39] = {
	-1,  0,  3,  3,  2,  8,  5,  5, -1, -1,
	-1, -1, -1, -1, -1, -1,  9, 10, 10,  1,
	7,   6,  6, -1, -1, -1, -1, -1, -1, 11,
	11, 12, 14, -1, -1, 13, 13,  4,  4
};

int code_vesa[85] = {
	-1, -1, -1, -1, 15, -1, -1, -1, -1, 16,
	-1, -1, -1, -1, 17, -1, 23, -1, -1, -1,
	-1, -1, 29, 18, -1, -1, -1, 32, 19, -1,
	-1, -1, 21, -1, -1, 22, -1, -1, -1, 20,
	-1, 30, 24, -1, -1, -1, -1, 25, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, 31, 26, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 27, 28, -1, 33};

enum hdmi_phypwr {
	HDMI_PHYPWRCMD_OFF = 0,
	HDMI_PHYPWRCMD_LDOON = 1,
	HDMI_PHYPWRCMD_TXON = 2
};

enum hdmi_pll_pwr {
	HDMI_PLLPWRCMD_ALLOFF = 0,
	HDMI_PLLPWRCMD_PLLONLY = 1,
	HDMI_PLLPWRCMD_BOTHON_ALLCLKS = 2,
	HDMI_PLLPWRCMD_BOTHON_NOPHYCLK = 3
};

enum hdmi_core_inputbus_width {
	HDMI_INPUT_8BIT = 0,
	HDMI_INPUT_10BIT = 1,
	HDMI_INPUT_12BIT = 2
};

enum hdmi_core_dither_trunc {
	HDMI_OUTPUTTRUNCATION_8BIT = 0,
	HDMI_OUTPUTTRUNCATION_10BIT = 1,
	HDMI_OUTPUTTRUNCATION_12BIT = 2,
	HDMI_OUTPUTDITHER_8BIT = 3,
	HDMI_OUTPUTDITHER_10BIT = 4,
	HDMI_OUTPUTDITHER_12BIT = 5
};

enum hdmi_core_deepcolor_ed {
	HDMI_DEEPCOLORPACKECTDISABLE = 0,
	HDMI_DEEPCOLORPACKECTENABLE = 1
};

enum hdmi_core_packet_mode {
	HDMI_PACKETMODERESERVEDVALUE = 0,
	HDMI_PACKETMODE24BITPERPIXEL = 4,
	HDMI_PACKETMODE30BITPERPIXEL = 5,
	HDMI_PACKETMODE36BITPERPIXEL = 6,
	HDMI_PACKETMODE48BITPERPIXEL = 7
};

enum hdmi_core_hdmi_dvi {
	HDMI_DVI	= 0,
	HDMI_HDMI	= 1
};

enum hdmi_core_tclkselclkmult {
	FPLL05IDCK  = 0,
	FPLL10IDCK  = 1,
	FPLL20IDCK  = 2,
	FPLL40IDCK  = 3
};

enum hdmi_core_fs {
	FS_32000 = 0,
	FS_44100 = 1
};

enum hdmi_core_layout {
	LAYOUT_2CH = 0,
	LAYOUT_8CH = 1
};

enum hdmi_core_cts_mode {
	CTS_MODE_HW = 0,
	CTS_MODE_SW = 1
};

enum hdmi_core_packet_ctrl {
	PACKETENABLE = 1,
	PACKETDISABLE = 0,
	PACKETREPEATON = 1,
	PACKETREPEATOFF = 0
};

/* INFOFRAME_AVI_ definitions */
enum hdmi_core_infoframe {
	INFOFRAME_AVI_DB1Y_RGB = 0,
	INFOFRAME_AVI_DB1Y_YUV422 = 1,
	INFOFRAME_AVI_DB1Y_YUV444 = 2,
	INFOFRAME_AVI_DB1A_ACTIVE_FORMAT_OFF = 0,
	INFOFRAME_AVI_DB1A_ACTIVE_FORMAT_ON =  1,
	INFOFRAME_AVI_DB1B_NO = 0,
	INFOFRAME_AVI_DB1B_VERT = 1,
	INFOFRAME_AVI_DB1B_HORI = 2,
	INFOFRAME_AVI_DB1B_VERTHORI = 3,
	INFOFRAME_AVI_DB1S_0 = 0,
	INFOFRAME_AVI_DB1S_1 = 1,
	INFOFRAME_AVI_DB1S_2 = 2,
	INFOFRAME_AVI_DB2C_NO = 0,
	INFOFRAME_AVI_DB2C_ITU601 = 1,
	INFOFRAME_AVI_DB2C_ITU709 = 2,
	INFOFRAME_AVI_DB2C_EC_EXTENDED = 3,
	INFOFRAME_AVI_DB2M_NO = 0,
	INFOFRAME_AVI_DB2M_43 = 1,
	INFOFRAME_AVI_DB2M_169 = 2,
	INFOFRAME_AVI_DB2R_SAME = 8,
	INFOFRAME_AVI_DB2R_43 = 9,
	INFOFRAME_AVI_DB2R_169 = 10,
	INFOFRAME_AVI_DB2R_149 = 11,
	INFOFRAME_AVI_DB3ITC_NO = 0,
	INFOFRAME_AVI_DB3ITC_YES = 1,
	INFOFRAME_AVI_DB3EC_XVYUV601 = 0,
	INFOFRAME_AVI_DB3EC_XVYUV709 = 1,
	INFOFRAME_AVI_DB3Q_DEFAULT = 0,
	INFOFRAME_AVI_DB3Q_LR = 1,
	INFOFRAME_AVI_DB3Q_FR = 2,
	INFOFRAME_AVI_DB3SC_NO = 0,
	INFOFRAME_AVI_DB3SC_HORI = 1,
	INFOFRAME_AVI_DB3SC_VERT = 2,
	INFOFRAME_AVI_DB3SC_HORIVERT = 3,
	INFOFRAME_AVI_DB5PR_NO = 0,
	INFOFRAME_AVI_DB5PR_2 = 1,
	INFOFRAME_AVI_DB5PR_3 = 2,
	INFOFRAME_AVI_DB5PR_4 = 3,
	INFOFRAME_AVI_DB5PR_5 = 4,
	INFOFRAME_AVI_DB5PR_6 = 5,
	INFOFRAME_AVI_DB5PR_7 = 6,
	INFOFRAME_AVI_DB5PR_8 = 7,
	INFOFRAME_AVI_DB5PR_9 = 8,
	INFOFRAME_AVI_DB5PR_10 = 9
};

enum hdmi_stereo_channel {
	HDMI_STEREO_NOCHANNEL = 0,
	HDMI_STEREO_ONECHANNELS = 1,
	HDMI_STEREO_TWOCHANNELS = 2,
	HDMI_STEREO_THREECHANNELS = 3,
	HDMI_STEREO_FOURCHANNELS = 4
};

enum hdmi_cea_code {
	HDMI_CEA_CODE_00 = 0x0,
	HDMI_CEA_CODE_01 = 0x1,
	HDMI_CEA_CODE_02 = 0x2,
	HDMI_CEA_CODE_03 = 0x3,
	HDMI_CEA_CODE_04 = 0x4,
	HDMI_CEA_CODE_05 = 0x5,
	HDMI_CEA_CODE_06 = 0x6,
	HDMI_CEA_CODE_07 = 0x7,
	HDMI_CEA_CODE_08 = 0x8,
	HDMI_CEA_CODE_09 = 0x9,
	HDMI_CEA_CODE_0A = 0xA,
	HDMI_CEA_CODE_0B = 0xB,
	HDMI_CEA_CODE_0C = 0xC,
	HDMI_CEA_CODE_0D = 0xD,
	HDMI_CEA_CODE_0E = 0xE,
	HDMI_CEA_CODE_0F = 0xF,
	HDMI_CEA_CODE_10 = 0x10,
	HDMI_CEA_CODE_11 = 0x11,
	HDMI_CEA_CODE_12 = 0x12,
	HDMI_CEA_CODE_13 = 0x13,
	HDMI_CEA_CODE_14 = 0x14,
	HDMI_CEA_CODE_15 = 0x15,
	HDMI_CEA_CODE_16 = 0x16,
	HDMI_CEA_CODE_17 = 0x17,
	HDMI_CEA_CODE_18 = 0x18,
	HDMI_CEA_CODE_19 = 0x19,
	HDMI_CEA_CODE_1A = 0x1A,
	HDMI_CEA_CODE_1B = 0x1B,
	HDMI_CEA_CODE_1C = 0x1C,
	HDMI_CEA_CODE_1D = 0x1D,
	HDMI_CEA_CODE_1E = 0x1E,
	HDMI_CEA_CODE_1F = 0x1F,
	HDMI_CEA_CODE_20 = 0x20,
	HDMI_CEA_CODE_21 = 0x21,
	HDMI_CEA_CODE_22 = 0x22,
	HDMI_CEA_CODE_23 = 0x23,
	HDMI_CEA_CODE_24 = 0x24,
	HDMI_CEA_CODE_25 = 0x25,
	HDMI_CEA_CODE_26 = 0x26
};

enum hdmi_iec_format {
	HDMI_AUDIO_FORMAT_LPCM = 0,
	HDMI_AUDIO_FORMAT_IEC = 1
};

enum hdmi_audio_justify {
	HDMI_AUDIO_JUSTIFY_LEFT = 0,
	HDMI_AUDIO_JUSTIFY_RIGHT = 1
};

enum hdmi_sample_order {
	HDMI_SAMPLE_RIGHT_FIRST = 0,
	HDMI_SAMPLE_LEFT_FIRST = 1
};

enum hdmi_sample_perword {
	HDMI_ONEWORD_ONE_SAMPLE = 0,
	HDMI_ONEWORD_TWO_SAMPLES = 1
};

enum hdmi_sample_size {
	HDMI_SAMPLE_16BITS = 0,
	HDMI_SAMPLE_24BITS = 1
};

enum hdmi_dma_irq {
	HDMI_THRESHOLD_DMA = 0,
	HDMI_THRESHOLD_IRQ = 1
};

enum hdmi_block_start_end {
	HDMI_BLOCK_STARTEND_ON = 0,
	HDMI_BLOCK_STARTEND_OFF = 1
};


enum hdmi_core_if_fs {
	IF_FS_NO     = 0x0,
	IF_FS_32000  = 0x1,
	IF_FS_44100  = 0x2,
	IF_FS_48000  = 0x3,
	IF_FS_88200  = 0x4,
	IF_FS_96000  = 0x5,
	IF_FS_176400 = 0x6,
	IF_FS_192000 = 0x7
};

enum hdmi_core_if_sample_size {
	IF_NO_PER_SAMPLE = 0x0,
	IF_16BIT_PER_SAMPLE = 0x1,
	IF_20BIT_PER_SAMPLE = 0x2,
	IF_24BIT_PER_SAMPLE = 0x3
};


enum hdmi_packing_mode {
	HDMI_PACK_10b_RGB_YUV444 = 0,
	HDMI_PACK_24b_RGB_YUV444_YUV422 = 1,
	HDMI_PACK_20b_YUV422 = 2,
	HDMI_PACK_ALREADYPACKED = 7
};


struct hdmi_core_video_config {
	enum hdmi_core_inputbus_width		ip_bus_width;
	enum hdmi_core_dither_trunc		op_dither_truc;
	enum hdmi_core_deepcolor_ed		deep_color_pkt;
	enum hdmi_core_packet_mode		pkt_mode;
	enum hdmi_core_hdmi_dvi			hdmi_dvi;
	enum hdmi_core_tclkselclkmult		tclk_sel_clkmult;
};

/*
 * Refer to section 8.2 in HDMI 1.3 specification for
 * details about infoframe databytes
 */
struct hdmi_core_infoframe_avi {
	u8		db1_format;
			/* Y0, Y1 rgb,yCbCr */
	u8		db1_active_info;
			/* A0  Active information Present */
	u8		db1_bar_info_dv;
			/* B0, B1 Bar info data valid */
	u8		db1_scan_info;
			/* S0, S1 scan information */
	u8		db2_colorimetry;
			/* C0, C1 colorimetry */
	u8		db2_aspect_ratio;
			/* M0, M1 Aspect ratio (4:3, 16:9) */
	u8		db2_active_fmt_ar;
			/* R0...R3 Active format aspect ratio */
	u8		db3_itc;
			/* ITC IT content. */
	u8		db3_ec;
			/* EC0, EC1, EC2 Extended colorimetry */
	u8		db3_q_range;
			/* Q1, Q0 Quantization range */
	u8		db3_nup_scaling;
			/* SC1, SC0 Non-uniform picture scaling */
	u8		db4_videocode;
			/* VIC0..6 Video format identification */
	u8		db5_pixel_repeat;
			/* PR0..PR3 Pixel repetition factor */
	u16		db6_7_line_eoftop;
			/* Line number end of top bar */
	u16		db8_9_line_sofbottom;
			/* Line number start of bottom bar */
	u16		db10_11_pixel_eofleft;
			/* Pixel number end of left bar */
	u16		db12_13_pixel_sofright;
			/* Pixel number start of right bar */
};

struct hdmi_core_packet_enable_repeat {
	u32		audio_pkt;
	u32		audio_pkt_repeat;
	u32		avi_infoframe;
	u32		avi_infoframe_repeat;
	u32		gen_cntrl_pkt;
	u32		gen_cntrl_pkt_repeat;
	u32		generic_pkt;
	u32		generic_pkt_repeat;
};

struct hdmi_audio_format {
	enum hdmi_stereo_channel	stereo_channel_enable;
	enum hdmi_cea_code		audio_channel_location;
	enum hdmi_iec_format		iec;
	enum hdmi_audio_justify		justify;
	enum hdmi_sample_order		left_before;
	enum hdmi_sample_perword	sample_number;
	enum hdmi_sample_size		sample_size;
};


struct hdmi_audio_dma {
	u8				dma_transfer;
	u8				block_size;
	enum hdmi_dma_irq		dma_or_irq;
	u16				threshold_value;
	enum hdmi_block_start_end	block_start_end;
};

struct hdmi_video_format {
	enum hdmi_packing_mode	packing_mode;
	u32			y_res;	/* Line per panel */
	u32			x_res;	/* pixel per line */
};

struct hdmi_video_interface {
	int			vsp;	/* Vsync polarity */
	int			hsp;	/* Hsync polarity */
	int			interlacing;
	int			tm;	/* Timing mode */
};

struct hdmi_video_timing {
	u32			hbp;
	u32			hfp;
	u32			hsw;
	u32			vbp;
	u32			vfp;
	u32			vsw;
};

struct hdmi_config {
	u16			ppl;	/* pixel per line */
	u16			lpp;	/* line per panel */
	u32			pixel_clock;
	u16			hsw;	/* Horizontal sync pulse width */
	u16			hfp;	/* Horizontal front porch */
	u16			hbp;	/* Horizontal back porch */
	u16			vsw;	/* Vertical sync pulse width */
	u16			vfp;	/* Vertical front porch */
	u16			vbp;	/* Vertical back porch */
	u16			interlace;
	u16			h_pol;
	u16			v_pol;
	u16			hdmi_dvi;
	u16			video_format;
};

struct hdmi_core_audio_config {
	enum hdmi_core_fs		fs; /* 0=32KHz - 1=44.1KHz */
	u32				n;
	u32				cts;
	u32				aud_par_busclk;
	enum hdmi_core_layout		layout; /* 0: 2Ch - 1: 8Ch */
	enum hdmi_core_cts_mode		cts_mode; /* 0: HW  - 1: SW*/
	enum hdmi_core_if_fs		if_fs;
	u32				if_channel_number;
	enum hdmi_core_if_sample_size	if_sample_size;
	enum hdmi_cea_code		if_audio_channel_location;
};

struct hdmi_cm {
	int code;
	int mode;
};

struct hdmi_irq_vector {
	u8	pll_recal;
	u8	pll_unlock;
	u8	pll_lock;
	u8	phy_disconnect;
	u8	phy_connect;
	u8	phy_short_5v;
	u8	video_end_fr;
	u8	video_vsync;
	u8	fifo_sample_req;
	u8	fifo_overflow;
	u8	fifo_underflow;
	u8	ocp_timeout;
	u8	core;
};

/* Function prototype */
int hdmi_wait_phy_pwr(int param);
int hdmi_wait_pll_pwr(int param);
int hdmi_wait_softreset(void);
int hdmi_core_ddc_edid(u8 *data, int ext);
void hdmi_lib_enable(struct hdmi_config *cfg);
int hdmi_lib_init(void);
void hdmi_lib_exit(void);
int hdmi_pll_program(struct hdmi_pll_info *fmt);

#endif

