/* linux/arch/arm/plat-s5p/dev-dsim.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DSIM controller configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


#include "dsim.h"
#include "mipi_ddi.h"
#include "s3cfb.h"
#define false 0
#define true 1

static struct dsim_config dsim_info = {
	.auto_flush = false,		/* main frame fifo auto flush at VSYNC pulse */

	.eot_disable = false,		/* only DSIM_1_02 or DSIM_1_03 */

	.auto_vertical_cnt = true,
	.hse = false,
	.hfp = false, //true
	.hbp = false,
	.hsa = false,

	.e_no_data_lane = DSIM_DATA_LANE_4,
	.e_byte_clk = DSIM_PLL_OUT_DIV8,

        .p = 3,
  //      .m = 63,
        .m = 52,
        .s = 0,
	.pll_stable_time = 300,		/* D-PHY PLL stable time spec :min = 200usec ~ max 400usec */

	.esc_clk = 15 * 1000000,	/* escape clk : 10MHz */

	.stop_holding_cnt = 0x07ff,	/* stop state holding counter after bta change count 0 ~ 0xfff */
	.bta_timeout = 0xff,		/* bta timeout 0 ~ 0xff */
	.rx_timeout = 0xffff,		/* lp rx timeout 0 ~ 0xffff */

	.e_lane_swap = DSIM_NO_CHANGE,
};

static struct s3cfb_lcd k3_mipi_lcd = {
	.width  = 720,
	.height = 1280,
	.bpp    = 24,
	.freq   = 60,

	.timing = {
		.h_fp = 0x10,
		.h_bp = 0x43,
		.h_sw = 0x11,
		.v_fp = 0x0b,
		.v_fpe = 1,
		.v_bp = 0x0b,
		.v_bpe = 1,
		.v_sw = 0x02,
		.cmd_allow_len = 0xf,
	},

	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 0,
		.inv_vsync = 0,
		.inv_vden = 0,
	},
};

static struct dsim_lcd_config dsim_lcd_info = {
	.e_interface		= DSIM_VIDEO,
	
	.parameter[DSI_VIRTUAL_CH_ID]	= (unsigned int) DSIM_VIRTUAL_CH_0,
	.parameter[DSI_FORMAT]		= (unsigned int) DSIM_24BPP_888,
	.parameter[DSI_VIDEO_MODE_SEL]	= (unsigned int) /*DSIM_NON_BURST_SYNC_PULSE*/DSIM_BURST,
	
	.lcd_panel_info =(void*)&k3_mipi_lcd,
};

static void s5p_dsim_mipi_power(int enable)
{
	return;
}

 struct s5p_platform_dsim dsim_platform_data = {
	.clk_name = "dsim0",
	.dsim_info = &dsim_info,
	.dsim_lcd_info = &dsim_lcd_info,
	.mipi_power = s5p_dsim_mipi_power,
	.part_reset = s5p_dsim_part_reset,
	.init_d_phy = s5p_dsim_init_d_phy,	
};


