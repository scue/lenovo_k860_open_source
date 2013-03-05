/* linux/drivers/video/samsung/s5p-dsim.c
 *
 * Samsung MIPI-DSIM driver.
 *
 * InKi Dae, <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Modified by Samsung Electronics (UK) on May 2010
 *
*/



#include "dsim.h"
#include "mipi_ddi.h"
#include "s5p-dsim.h"
#include "s5p_dsim_lowlevel.h"
#include "s3cfb.h"
#include "regs-dsim.h"
#include <asm/io.h>

#define GFP_KERNEL 0
#define printk printf
#define mdelay(n) udelay(n*1000)

int lcd_id;
extern struct mipi_lcd_driver otm1280a_mipi_driver;

/* Indicates the state of the device */
struct dsim_global {
	struct s5p_platform_dsim *pd;
	struct dsim_config *dsim_info;
	struct dsim_lcd_config *dsim_lcd_info;
	/* lcd panel data. */
	struct s3cfb_lcd *lcd_panel_info;
	/* platform and machine specific data for lcd panel driver. */
	struct mipi_ddi_platform_data *mipi_ddi_pd;
	/* lcd panel driver based on MIPI-DSI. */
	struct mipi_lcd_driver *mipi_drv;

	unsigned int irq;
	unsigned int te_irq;
	unsigned int reg_base;

	unsigned char state;
	unsigned int data_lane;
	enum dsim_byte_clk_src e_clk_src;
	unsigned long hs_clk;
	unsigned long byte_clk;
	unsigned long escape_clk;
	unsigned char freq_band;

	char header_fifo_index[DSIM_HEADER_FIFO_SZ];
};

static struct dsim_global dsim;

unsigned char s5p_dsim_wr_data(unsigned int dsim_base,
	unsigned int data_id, unsigned int data0, unsigned int data1)
{
	unsigned char check_rx_ack = 0;

	if (dsim.state == DSIM_STATE_ULPS) {
		printf( "state is ULPS.\n");
		return DSIM_FALSE;
	}
	switch (data_id) {
	/* short packet types of packet types for command. */
	case GEN_SHORT_WR_NO_PARA:
	case GEN_SHORT_WR_1_PARA:
	case GEN_SHORT_WR_2_PARA:
	case DCS_WR_NO_PARA:
	case DCS_WR_1_PARA:
	case SET_MAX_RTN_PKT_SIZE:
		s5p_dsim_wr_tx_header(dsim_base, (unsigned char) data_id,
			(unsigned char) data0, (unsigned char) data1);
		if (check_rx_ack)
			/* process response func  should be implemented */
			return DSIM_TRUE;
		else
			return DSIM_TRUE;

	/* general command */
	case CMD_OFF:
	case CMD_ON:
	case SHUT_DOWN:
	case TURN_ON:
		s5p_dsim_wr_tx_header(dsim_base, (unsigned char) data_id,
			(unsigned char) data0, (unsigned char) data1);
		if (check_rx_ack)
			/* process response func should be implemented. */
			return DSIM_TRUE;
		else
			return DSIM_TRUE;

	/* packet types for video data */
	case VSYNC_START:
	case VSYNC_END:
	case HSYNC_START:
	case HSYNC_END:
	case EOT_PKT:
		return DSIM_TRUE;

	/* short and response packet types for command */
	case GEN_RD_1_PARA:
	case GEN_RD_2_PARA:
	case GEN_RD_NO_PARA:
	case DCS_RD_NO_PARA:
		s5p_dsim_clear_interrupt(dsim_base, 0xffffffff);
		s5p_dsim_wr_tx_header(dsim_base, (unsigned char) data_id,
			(unsigned char) data0, (unsigned char) data1);
		/* process response func should be implemented. */
		return DSIM_FALSE;

	/* long packet type and null packet */
	case NULL_PKT:
	case BLANKING_PKT:
		return DSIM_TRUE;
	case GEN_LONG_WR:
	case DCS_LONG_WR:
		{
			u32 uCnt = 0;
			u32* pWordPtr = (u32 *)data0;
			do {
				s5p_dsim_wr_tx_data(dsim_base, pWordPtr[uCnt]);
				udelay(100);
			} while (((data1-1) / 4) > uCnt++);
				mdelay(10);
		}

		/* put data into header fifo */
		s5p_dsim_wr_tx_header(dsim_base, (unsigned char)data_id,
			(unsigned char)(((unsigned short)data1) & 0xff),
			(unsigned char)((((unsigned short)data1) & 0xff00) >> 8));

		if (check_rx_ack)
			/* process response func should be implemented. */
			return DSIM_TRUE;
		else
			return DSIM_TRUE;

	/* packet typo for video data */
	case RGB565_PACKED:
	case RGB666_PACKED:
	case RGB666_LOOSLY:
	case RGB888_PACKED:
		if (check_rx_ack)
			/* process response func should be implemented. */
			return DSIM_TRUE;
		else
			return DSIM_TRUE;
	default:
		printf("data id %x is not supported current DSI spec.\n", data_id);
		return DSIM_FALSE;
	}

	return DSIM_TRUE;
}

static void s5p_dsim_init_header_fifo(void)
{
	unsigned int cnt;

	for (cnt = 0; cnt < DSIM_HEADER_FIFO_SZ; cnt++)
		dsim.header_fifo_index[cnt] = -1;
	return;
}

unsigned char s5p_dsim_pll_on(unsigned int dsim_base, unsigned char enable)
{
	if (enable) {
		int sw_timeout = 1000;
		s5p_dsim_clear_interrupt(dsim_base, DSIM_PLL_STABLE);
		s5p_dsim_enable_pll(dsim_base, 1);
		while (1) {
			sw_timeout--;
			if (s5p_dsim_is_pll_stable(dsim_base))
				return DSIM_TRUE;
			if (sw_timeout == 0)
				return DSIM_FALSE;
		}
	} else
		s5p_dsim_enable_pll(dsim_base, 0);

	return DSIM_TRUE;
}

static unsigned long s5p_dsim_change_pll(unsigned int dsim_base, unsigned char pre_divider,
	unsigned short main_divider, unsigned char scaler)
{
	unsigned long dfin_pll, dfvco, dpll_out;
	unsigned char freq_band;
	unsigned char temp0, temp1;

	dfin_pll = (MIPI_FIN / pre_divider);

	if (dfin_pll < 6 * 1000 * 1000 || dfin_pll > 12 * 1000 * 1000) {
		printf( "warning!!\n");
		printf( "fin_pll range is 6MHz ~ 12MHz\n");
		printf( "fin_pll of mipi dphy pll is %luMHz\n",
				(dfin_pll / 1000000));

		s5p_dsim_enable_afc(dsim_base, 0, 0);
	} else {
		if (dfin_pll < 7 * 1000000)
			s5p_dsim_enable_afc(dsim_base, 1, 0x1);
		else if (dfin_pll < 8 * 1000000)
			s5p_dsim_enable_afc(dsim_base, 1, 0x0);
		else if (dfin_pll < 9 * 1000000)
			s5p_dsim_enable_afc(dsim_base, 1, 0x3);
		else if (dfin_pll < 10 * 1000000)
			s5p_dsim_enable_afc(dsim_base, 1, 0x2);
		else if (dfin_pll < 11 * 1000000)
			s5p_dsim_enable_afc(dsim_base, 1, 0x5);
		else
			s5p_dsim_enable_afc(dsim_base, 1, 0x4);
	}

	dfvco = dfin_pll * main_divider;
	printf( "dfvco = %lu, dfin_pll = %lu, main_divider = %d\n",
		dfvco, dfin_pll, main_divider);
	if (dfvco < 500000000 || dfvco > 1000000000) {
		printf( "Caution!!\n");
		printf( "fvco range is 500MHz ~ 1000MHz\n");
		printf( "fvco of mipi dphy pll is %luMHz\n",
				(dfvco / 1000000));
	}

	dpll_out = dfvco / (1 << scaler);
	printf("dpll_out = %lu, dfvco = %lu, scaler = %d\n",
		dpll_out, dfvco, scaler);
	if (dpll_out < 100 * 1000000)
		freq_band = 0x0;
	else if (dpll_out < 120 * 1000000)
		freq_band = 0x1;
	else if (dpll_out < 170 * 1000000)
		freq_band = 0x2;
	else if (dpll_out < 220 * 1000000)
		freq_band = 0x3;
	else if (dpll_out < 270 * 1000000)
		freq_band = 0x4;
	else if (dpll_out < 320 * 1000000)
		freq_band = 0x5;
	else if (dpll_out < 390 * 1000000)
		freq_band = 0x6;
	else if (dpll_out < 450 * 1000000)
		freq_band = 0x7;
	else if (dpll_out < 510 * 1000000)
		freq_band = 0x8;
	else if (dpll_out < 560 * 1000000)
		freq_band = 0x9;
	else if (dpll_out < 640 * 1000000)
		freq_band = 0xa;
	else if (dpll_out < 690 * 1000000)
		freq_band = 0xb;
	else if (dpll_out < 770 * 1000000)
		freq_band = 0xc;
	else if (dpll_out < 870 * 1000000)
		freq_band = 0xd;
	else if (dpll_out < 950 * 1000000)
		freq_band = 0xe;
	else
		freq_band = 0xf;

	printf( "freq_band = %d\n", freq_band);

	s5p_dsim_pll_freq(dsim_base, pre_divider, main_divider, scaler);
	temp0 = 0;
	s5p_dsim_hs_zero_ctrl(dsim_base, temp0);
	temp1 = 0;
	s5p_dsim_prep_ctrl(dsim_base, temp1);

	/* Freq Band */
	s5p_dsim_pll_freq_band(dsim_base, freq_band);

	/* Stable time */
	s5p_dsim_pll_stable_time(dsim_base, dsim.dsim_info->pll_stable_time);

	/* Enable PLL */
	printf( "FOUT of mipi dphy pll is %luMHz\n",
			(dpll_out / 1000000));

	return dpll_out;
}

static void s5p_dsim_set_clock(unsigned int dsim_base,
	unsigned char byte_clk_sel, unsigned char enable)
{
	unsigned int esc_div;
	unsigned long esc_clk_error_rate;

	if (enable) {
		dsim.e_clk_src = byte_clk_sel;

		/* Escape mode clock and byte clock source */
		s5p_dsim_set_byte_clock_src(dsim_base, byte_clk_sel);

		/* DPHY, DSIM Link : D-PHY clock out */
		if (byte_clk_sel == DSIM_PLL_OUT_DIV8) {
			dsim.hs_clk = s5p_dsim_change_pll(dsim_base,
					dsim.dsim_info->p, dsim.dsim_info->m,
					dsim.dsim_info->s);
			dsim.byte_clk = dsim.hs_clk / 8;
			s5p_dsim_enable_pll_bypass(dsim_base, 0);
			s5p_dsim_pll_on(dsim_base, 1);
		/* DPHY : D-PHY clock out, DSIM link : external clock out */
		} else if (byte_clk_sel == DSIM_EXT_CLK_DIV8)
			printf( "this project is not supported "
				"external clock source for MIPI DSIM\n");
		else if (byte_clk_sel == DSIM_EXT_CLK_BYPASS)
			printf( "this project is not supported "
				"external clock source for MIPI DSIM\n");

		/* escape clock divider */
		esc_div = dsim.byte_clk / (dsim.dsim_info->esc_clk);
		printf("esc_div = %d, byte_clk = %lu, "
				"esc_clk = %lu\n",
			esc_div, dsim.byte_clk, dsim.dsim_info->esc_clk);
		if ((dsim.byte_clk / esc_div) >= 20000000 ||
			(dsim.byte_clk / esc_div) > dsim.dsim_info->esc_clk)
			esc_div += 1;

		dsim.escape_clk = dsim.byte_clk / esc_div;
		printf( "escape_clk = %lu, byte_clk = %lu, "
				"esc_div = %d\n",
			dsim.escape_clk, dsim.byte_clk, esc_div);

		/*
		 * enable escclk on lane
		 */
		s5p_dsim_enable_byte_clock(dsim_base, DSIM_TRUE);

		/* enable byte clk and escape clock */
		s5p_dsim_set_esc_clk_prs(dsim_base, 1, esc_div);
		/* escape clock on lane */
		s5p_dsim_enable_esc_clk_on_lane(dsim_base, (DSIM_LANE_CLOCK | dsim.data_lane), 1);

		printf( "byte clock is %luMHz\n",
				(dsim.byte_clk / 1000000));
		printf( "escape clock that user's need is %lu\n",
				(dsim.dsim_info->esc_clk / 1000000));
		printf( "escape clock divider is %x\n", esc_div);
		printf("escape clock is %luMHz\n",
				((dsim.byte_clk / esc_div) / 1000000));

		if ((dsim.byte_clk / esc_div) > dsim.escape_clk) {
			esc_clk_error_rate =
				dsim.escape_clk / (dsim.byte_clk / esc_div);
			printf( "error rate is %lu over.\n",
					(esc_clk_error_rate / 100));
		} else if ((dsim.byte_clk / esc_div) < (dsim.escape_clk)) {
			esc_clk_error_rate =
				(dsim.byte_clk / esc_div) / dsim.escape_clk;
			printf("error rate is %lu under.\n",
					(esc_clk_error_rate / 100));
		}
	} else {
		s5p_dsim_enable_esc_clk_on_lane(dsim_base,
				(DSIM_LANE_CLOCK | dsim.data_lane), 0);
		s5p_dsim_set_esc_clk_prs(dsim_base, 0, 0);

		s5p_dsim_enable_byte_clock(dsim_base, DSIM_FALSE);

		if (byte_clk_sel == DSIM_PLL_OUT_DIV8)
			s5p_dsim_pll_on(dsim_base, 0);
	}
}


static int s5p_dsim_init_dsim(unsigned int dsim_base)
{
	if (dsim.pd->init_d_phy)
		dsim.pd->init_d_phy(dsim.reg_base);

	if (dsim.pd->cfg_gpio)
		dsim.pd->cfg_gpio();

	dsim.state = DSIM_STATE_RESET;

	switch (dsim.dsim_info->e_no_data_lane) {
	case DSIM_DATA_LANE_1:
		dsim.data_lane = DSIM_LANE_DATA0;
		break;
	case DSIM_DATA_LANE_2:
		printk("DSIM_DATA_LANE_2\n");
		dsim.data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1;
		break;
	case DSIM_DATA_LANE_3:
		dsim.data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1 |
			DSIM_LANE_DATA2;
		break;
	case DSIM_DATA_LANE_4:
		printk("DSIM_DATA_LANE_4\n");
		dsim.data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1 |
			DSIM_LANE_DATA2 | DSIM_LANE_DATA3;
		break;
	default:
		printf("data lane is invalid.\n");
		return -1;
	};

	s5p_dsim_init_header_fifo();
	/* s5p_dsim_sw_reset(dsim_base); */
	s5p_dsim_dp_dn_swap(dsim_base, dsim.dsim_info->e_lane_swap);

	/* enable only frame done interrupt */
	/* s5p_dsim_clear_interrupt(dsim_base, AllDsimIntr); */
	s5p_dsim_set_interrupt_mask(dsim.reg_base, AllDsimIntr, 1);

	return 0;
}

static void s5p_dsim_set_display_mode(unsigned int dsim_base,
	struct dsim_lcd_config *main_lcd, struct dsim_lcd_config *sub_lcd)
{
	struct s3cfb_lcd *main_lcd_panel_info = NULL, *sub_lcd_panel_info = NULL;
	struct s3cfb_lcd_timing *main_timing = NULL;

	if (main_lcd != NULL) {
		if (main_lcd->lcd_panel_info != NULL) {
			main_lcd_panel_info =
				(struct s3cfb_lcd *) main_lcd->lcd_panel_info;

			s5p_dsim_set_main_disp_resol(dsim_base,
				main_lcd_panel_info->height,
				main_lcd_panel_info->width);
		} else
			printf( "lcd panel info of main lcd is NULL.\n");
	} else {
		printf("main lcd is NULL.\n");
		return;
	}

	/* in case of VIDEO MODE (RGB INTERFACE) */
	if (dsim.dsim_lcd_info->e_interface == (u32) DSIM_VIDEO) {

		main_timing = &main_lcd_panel_info->timing;
		if (main_timing == NULL) {
			printf("main_timing is NULL.\n");
			return;
		}

		/* if (dsim.dsim_info->auto_vertical_cnt == DSIM_FALSE) */
		{
			s5p_dsim_set_main_disp_vporch(dsim_base,
				main_timing->cmd_allow_len,
				main_timing->v_fp, (u16) main_timing->v_bp);
			s5p_dsim_set_main_disp_hporch(dsim_base,
				main_timing->h_fp, (u16) main_timing->h_bp);
			s5p_dsim_set_main_disp_sync_area(dsim_base,
				main_timing->v_sw, (u16) main_timing->h_sw);
		}
	/* in case of COMMAND MODE (CPU or I80 INTERFACE) */
	} else {
		if (sub_lcd != NULL) {
			if (sub_lcd->lcd_panel_info != NULL) {
				sub_lcd_panel_info =
					(struct s3cfb_lcd *)
						sub_lcd->lcd_panel_info;

				s5p_dsim_set_sub_disp_resol(dsim_base,
					sub_lcd_panel_info->height,
					sub_lcd_panel_info->width);
			} else
				printf("lcd panel info of sub lcd is NULL.\n");
		}
	}

	s5p_dsim_display_config(dsim_base, dsim.dsim_lcd_info, NULL);
}

static int s5p_dsim_init_link(unsigned int dsim_base)
{
	unsigned int time_out = 100;

	switch (dsim.state) {
	case DSIM_STATE_RESET:
		s5p_dsim_sw_reset(dsim_base);
	case DSIM_STATE_INIT:
		printf("KKKKKKKKKKKKKKKKKKKKKK %s \n", __func__);
		s5p_dsim_init_fifo_pointer(dsim_base, 0x0);
		mdelay(10);
		s5p_dsim_init_fifo_pointer(dsim_base, 0x1f);

		/* dsi configuration */
		s5p_dsim_init_config(dsim_base, dsim.dsim_lcd_info,
				NULL, dsim.dsim_info);
		s5p_dsim_enable_lane(dsim_base, DSIM_LANE_CLOCK, 1);
		s5p_dsim_enable_lane(dsim_base, dsim.data_lane, 1);

		/* set clock configuration */
		s5p_dsim_set_clock(dsim_base, dsim.dsim_info->e_byte_clk, 1);

		/* check clock and data lane state is stop state */
		while (!(s5p_dsim_is_lane_state(dsim_base, DSIM_LANE_CLOCK)
				== DSIM_LANE_STATE_STOP) &&
			!(s5p_dsim_is_lane_state(dsim_base, dsim.data_lane)
				== DSIM_LANE_STATE_STOP)) {
			time_out--;
			if (time_out == 0) {
				printf( "DSI Master state is not "
						"stop state!!!\n");
				printf( "Please check "
						"initialization process\n");

				return DSIM_FALSE;
			}
		}

		if (time_out != 0) {
			printf( "initialization of DSI Master is successful\n");
			printf( "DSI Master state is stop state\n");
		}
		dsim.state = DSIM_STATE_STOP;

		/* BTA sequence counters */
		s5p_dsim_set_stop_state_counter(dsim_base,
				dsim.dsim_info->stop_holding_cnt);
		s5p_dsim_set_bta_timeout(dsim_base,
				dsim.dsim_info->bta_timeout);
		s5p_dsim_set_lpdr_timeout(dsim_base,
				dsim.dsim_info->rx_timeout);

		/* default LPDT by both cpu and lcd controller */
		s5p_dsim_set_data_mode(dsim_base, DSIM_TRANSFER_BOTH,
				DSIM_STATE_STOP);

		return DSIM_TRUE;
	default:
		printf( "DSI Master is already init.\n");

		return DSIM_FALSE;
	}
}

unsigned char s5p_dsim_set_hs_enable(unsigned int dsim_base)
{
	u8 ret =  DSIM_FALSE;

	if (dsim.state == DSIM_STATE_STOP) {
		if (dsim.e_clk_src != DSIM_EXT_CLK_BYPASS) {
			dsim.state = DSIM_STATE_HSCLKEN;
			s5p_dsim_set_data_mode(dsim_base, DSIM_TRANSFER_BOTH,
				DSIM_STATE_HSCLKEN);
//			s5p_dsim_set_data_mode(dsim_base, DSIM_TRANSFER_BYLCDC,
//				DSIM_STATE_HSCLKEN);
	//		s5p_dsim_enable_hs_clock(dsim_base, 1);

			ret = DSIM_TRUE;
		} else
			printf( "clock source is external bypass.\n");
	} else
		printf("DSIM is not stop state.\n");

	return ret;
}


static unsigned char s5p_dsim_set_data_transfer_mode(unsigned int dsim_base,
	unsigned char data_path, unsigned char hs_enable)
{
	u8 ret = DSIM_FALSE;

	if (hs_enable) {
		if (dsim.state == DSIM_STATE_HSCLKEN) {
			s5p_dsim_set_data_mode(dsim_base, data_path, DSIM_STATE_HSCLKEN);
			ret = DSIM_TRUE;
		} else {
			printf( "HS Clock lane is not enabled.\n");
			ret = DSIM_FALSE;
		}
	} else {
		if (dsim.state == DSIM_STATE_INIT || dsim.state == DSIM_STATE_ULPS) {
			printf("DSI Master is not STOP or HSDT state.\n");
			ret = DSIM_FALSE;
		} else {
			s5p_dsim_set_data_mode(dsim_base, data_path, DSIM_STATE_STOP);
			ret = DSIM_TRUE;
		}
	}

	return ret;
}

extern  struct s5p_platform_dsim dsim_platform_data;
static struct s3cfb_lcd k3_mipi_lcd_boe = {
	.width  = 720,
	.height = 1280,
	.bpp    = 24,
	.freq   = 70,

	.timing = {
		.h_fp = 0x20,
		.h_bp = 0x3b,
		.h_sw = 0x09,
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
static struct s3cfb_lcd k3_mipi_lcd_lg = { 
	.width  = 720,
	.height = 1280,
	.bpp    = 24, 
	.freq   = 70, 

	.timing = { 
		.h_fp = 14, 
		.h_bp = 112,
		.h_sw = 4,
		.v_fp = 8, 
		.v_fpe = 1,
		.v_bp = 12, 
		.v_bpe = 1,
		.v_sw = 2,
		.cmd_allow_len = 0xf,
	},  

	.polarity = { 
		.rise_vclk = 0,
		.inv_hsync = 0,
		.inv_vsync = 0,
		.inv_vden = 0,
	},  
};
void gpio_cfg(void)
{
	printk("GPX3CON = 0x%x\n", readl(0x11000c60));
	printk("GPX3_3 set output\n");
	//output
	writel((readl(0x11000c60) & 0xfff0fff | (0x01 << 12)), 0x11000c60);
	printk("GPX3CON = 0x%x\n", readl(0x11000c60));
	printk("GPX3PUD = 0x%x\n", readl(0x11000c68));
	//pull_up enable
	writel((readl(0x11000c68) | (0x3 << 6)), 0x11000c68);
	printk("GPX3PUD = 0x%x\n", readl(0x11000c68));
	//output hight level
	printk("GPX3_3 output hight level\n");
	writel((readl(0x11000c64) | (0x01 << 3)), 0x11000c64);
	printk("GPX3DAT = 0x%x\n", readl(0x11000c64));
	mdelay(1);
	printk("GPX3_3 set input\n");
	writel((readl(0x11000c68) & ~(0x3 << 6)), 0x11000c68);
	printk("GPX3PUD = 0x%x\n", readl(0x11000c68));
	writel((readl(0x11000c60) & 0xfff0fff), 0x11000c60);
	printk("GPX3CON = 0x%x\n", readl(0x11000c60));
	mdelay(5);
	if ((readl(0x11000C64) >> 3) & 0x01) 
		lcd_id = LCD_LG;
	else
		lcd_id = LCD_BOE;
}
 int s5p_dsim_probe(void)
{
	int ret = -1;
	
	gpio_cfg();
	dsim.pd = (void *)&dsim_platform_data;
	/* set dsim config data, dsim lcd config data and lcd panel data. */
	dsim.dsim_info = dsim.pd->dsim_info;
	dsim.dsim_lcd_info = dsim.pd->dsim_lcd_info;
	if (lcd_id == LCD_LG) {
		printk("%s lcd_id = LCD_LG\n", __func__);
		dsim.lcd_panel_info = &k3_mipi_lcd_lg; 
		dsim.dsim_info->m = 56;
		dsim.dsim_info->pll_stable_time = 300; 
		dsim.dsim_info->esc_clk = 15 * 1000000;
	} else {
		printk("%s lcd_id = LCD_BOE\n", __func__);
		dsim.lcd_panel_info = &k3_mipi_lcd_boe; 
		dsim.dsim_info->m = 63;
		dsim.dsim_info->pll_stable_time = 500; 
		dsim.dsim_info->esc_clk = 20 * 1000000;                      
	}
	dsim.mipi_ddi_pd =
		(struct mipi_ddi_platform_data *)dsim.dsim_lcd_info->mipi_ddi_pd;

	/* clock */
	writel(readl(0x1003c234) &(~(0xf << 12))|(0x1 << 12) , 0x1003c234);//source clock usb XTIusb 24M
	writel(readl(0x1003c934) &(~(0x1 << 3))|(0x1 << 3) , 0x1003c934);//mipi clk enable
	writel(readl(0x10010210) |(0x3 << 0) , 0x10010210);


	/* ioremap for register block */
	dsim.reg_base = (unsigned int)0x11c80000;

	/* find lcd panel driver registered to mipi-dsi driver. */
	dsim.mipi_drv = &otm1280a_mipi_driver;// scan_mipi_driver(dsim.pd->lcd_panel_name);
	if (dsim.mipi_drv == NULL) {
		printf( "mipi_drv is NULL.\n");
		goto mipi_drv_err;
	}

	/* set lcd panel driver link */
	dsim.mipi_drv->set_link((void *) dsim.mipi_ddi_pd, dsim.reg_base,
		s5p_dsim_wr_data, NULL);

	s5p_dsim_init_dsim(dsim.reg_base);
	s5p_dsim_init_link(dsim.reg_base);
	s5p_dsim_set_hs_enable(dsim.reg_base);
	if (lcd_id == LCD_LG)
		s5p_dsim_enable_hs_clock(dsim.reg_base, 1);
	s5p_dsim_set_data_transfer_mode(dsim.reg_base, DSIM_TRANSFER_BYCPU, 1);
	mdelay(10);
	/* initialize lcd panel */
	if (dsim.mipi_drv->init) {
		dsim.mipi_drv->init();
		
#if 0
	int i = 0;
	for (i = 0; i < 24; i++)
        {
                printf("%#x=%#x\n", dsim.reg_base + i*4, readl(dsim.reg_base + i*4));
        }
#endif
	} 

	if (dsim.mipi_drv->display_on)
		dsim.mipi_drv->display_on();

	s5p_dsim_set_hs_enable(dsim.reg_base);
	s5p_dsim_enable_hs_clock(dsim.reg_base, 1);
	s5p_dsim_set_display_mode(dsim.reg_base, dsim.dsim_lcd_info, NULL);
	s5p_dsim_set_data_transfer_mode(dsim.reg_base, DSIM_TRANSFER_BYLCDC, 1);
	//mdelay(400);

	return 0;
mipi_drv_err:
	return -1;

}


