/* linux/drivers/video/samsung/s3cfb_dummymipilcd.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Modified by Samsung Electronics (UK) on May 2010
 *
 */

#include <malloc.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/mtd/compat.h>
#include "regs-dsim.h"
#include "dsim.h"
#include "mipi_ddi.h"
#include "s5p-dsim.h"
#include "s3cfb.h"
#include <asm/io.h>
#include <asm/arch/pmic.h>

#define LCD_CE_INTERFACE
#ifdef LCD_CE_INTERFACE
static int ce_mode = 1;
#endif
#define printk printf
#define mdelay(n) udelay(n*1000)
extern int lcd_id;
static struct mipi_ddi_platform_data *ddi_pd;

void otm1280a_write_0(unsigned char dcs_cmd)
{
	ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_NO_PARA, dcs_cmd, 0);
}

static void otm1280a_write_1(unsigned char dcs_cmd, unsigned char param1)
{
	ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, dcs_cmd, param1);
}

static void otm1280a_write_3(unsigned char dcs_cmd, unsigned char param1, unsigned char param2, unsigned char param3)
{
	unsigned char buf[4];
	buf[0] = dcs_cmd;
	buf[1] = param1;
	buf[2] = param2;
	buf[3] = param3;

	ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, 4);
}

static void otm1280a_write(void)
{
	unsigned char buf[15] = {0xf8, 0x01, 0x27, 0x27, 0x07, 0x07, 0x54,
		0x9f, 0x63, 0x86, 0x1a,
		0x33, 0x0d, 0x00, 0x00};
	ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
}

static void otm1280a_display_off(void)
{
	ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, 0x28, 0x00);
}

void otm1280a_sleep_in(void)
{
	ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_NO_PARA, 0x10, 0);
}

void otm1280a_sleep_out(void)
{
	ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_NO_PARA, 0x11, 0);
}

static void otm1280a_display_on(void)
{         
	int ret;
	if (lcd_id == LCD_LG) {
#if 0 
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, 0x51, current_brightness);
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, 0x53, 0x24);
#else

		otm1280a_write_1(0xc2, 0x02);
		mdelay(15);
		otm1280a_write_1(0xc2, 0x06);
		mdelay(15);
		otm1280a_write_1(0xc2, 0x4E);
		mdelay(90);
		otm1280a_write_0(0x11);
		mdelay(20);
		do{
			unsigned char buf[] = {0xF9,0x80,0x00};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);
		mdelay(15);
		otm1280a_write_0(0x29);
		mdelay(10);
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, 0x51, 0x30);
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, 0x53, 0x24);
#endif
	} else {
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, 0x51, 0x30);
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, 0x53, 0x24);
	}
}

void lcd_reset(void)
{
	u8 read_data;
	IIC0_ERead(0x12, 0x47, &read_data);
	IIC0_EWrite(0x12, 0x47,  (read_data | (0x3 << 6)));

	if (lcd_id == LCD_BOE) {
		printf("lcd reset for BOE\n");
		writel((readl(0x11400188) & (~(0x3 <<0))), 0x11400188);//GPF0-0
		writel((readl(0x11400180) & (~(0xf <<0)))|(0x1 <<0), 0x11400180);

		writel((readl(0x11400184) & (~(0x1 <<0)))|(0x1 <<0), 0x11400184);//output 1
		mdelay(1);   

		writel((readl(0x11400184) & (~(0x1 <<0))), 0x11400184);//output 0
		mdelay(20);               

		writel((readl(0x11400184) & (~(0x1 <<0)))|(0x1 <<0), 0x11400184);//output 1
		mdelay(20);    
	} else {
		printf("lcd reset for LG\n");
		writel((readl(0x11400188) & (~(0x3 <<0))), 0x11400188);//GPF0-0
		writel((readl(0x11400180) & (~(0xf <<0)))|(0x1 <<0), 0x11400180);

		writel((readl(0x11400184) & (~(0x1 <<0)))|(0x1 <<0), 0x11400184);//output 1
		mdelay(100);   

		writel((readl(0x11400184) & (~(0x1 <<0))), 0x11400184);//output 0
		mdelay(100);               

		writel((readl(0x11400184) & (~(0x1 <<0)))|(0x1 <<0), 0x11400184);//output 1
		mdelay(100);    

	}
}
static int lcd_init_cmd_write(unsigned int dsim_base, unsigned int data_id, unsigned char * buf, unsigned int size)
{
	int ret = DSIM_FALSE;
	u32 read_buf[100];

	memset(read_buf, 0, sizeof(read_buf));
	ret = ddi_pd->cmd_write(ddi_pd->dsim_base, data_id,  (unsigned int )buf, size);
	if (ret == 0){
		printk("cmmd_write error\n");
		return DSIM_FALSE;
	}

	return DSIM_TRUE;

}
#ifdef LCD_CE_INTERFACE
static int ce_on_off(int value)
{
	int ret;
	if (lcd_id == LCD_LG) {
		if (value) {
			printk("%s: CE on for LG.\n", __func__);
#if 1
			//otm1280a_write_1(0x70, 0x00);
#if 1
			do{
				unsigned char buf[] = {0x74,0x05,0x03, 0x85};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);
#endif

			do{
				unsigned char buf[] = {0x75,0x03,0x00,0x03};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x76,0x07,0x00,0x03};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

#endif
		} else {
			printk("%s: CE off.\n", __func__);
#if 1
			//otm1280a_write_1(0x70, 0x00);
#if 1
			do{
				unsigned char buf[] = {0x74,0x00,0x00,0x06};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);
#endif

			do{
				unsigned char buf[] = {0x75,0x00,0x00,0x07};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x76,0x00,0x00,0x06};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

#endif

		}
	} else {
#if 0
		if (value) {
			printk("%s boe ce on, value = %d\n", __func__, value);
			otm1280a_write_1(0xe6, 0x01);
			otm1280a_write_1(0xe4, value);
		} else {
			printk("%s boe ce off\n", __func__);
			otm1280a_write_1(0xe6, 0x00);
			otm1280a_write_1(0xe4, 0x00);
		}
#endif
	}

	return 0;
}
#endif

int lcd_pannel_on(void)
{
	int ret = DSIM_TRUE;
	lcd_reset();
	if (lcd_id == LCD_BOE) {
		printk("%s BOE\n", __func__);
		otm1280a_write_0(0x11);
		mdelay(200);

		do{
			unsigned char buf[] = {0xB9,0xFF,0x83,0x94};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);

		do{
			unsigned char buf[] = {0xBA,0x13};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);
#if 0
		do{
			//unsigned char buf[] = {0xB1,0x7C,0x00,0x34,0x09,0x01,0x11,0x11,0x36,0x3E,0x26,0x26,0x57,0x12,0x01,0xE6};
			unsigned char buf[] = {0xB1,0x7C,0x00,0x34,0x09,0x01,0x11,0x11,0x36,0x3e,0x26,0x26,0x57,0x12,0x01,0xe6};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);
#else
		do{
			unsigned char buf[] = {0xB1,0x7C,0x00,0x34,0x09,0x01,0x11,0x11,0x36,0x3e,0x26,0x26,0x57,0x12,0x01,0xe6};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
		}while(0);
#endif
#if 0

		do{
			unsigned char buf[] = {0xB4,0x00,0x00,0x00,0x05,0x06,0x41,0x42,0x02,0x41,0x42,0x43,0x47,0x19,0x58,0x60,0x08,0x85,0x10};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);
#else
		do{
			unsigned char buf[] = {0xB4,0x00,0x00,0x00,0x05,0x06,0x41,0x42,0x02,0x41,0x42,0x43,0x47,0x19,0x58,0x60,0x08,0x85,0x10};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
		}while(0);
#endif
#if 0
		do{
			unsigned char buf[] = {0xD5,0x4C,0x01,0x00,0x01,0xCD,0x23,0xEF,0x45,0x67,0x89,0xAB,0x11,0x00,0xDC,0x10,0xFE,0x32,0xBA,0x98,0x76,0x54,0x00,0x11,0x40};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);
#else
		do{
			unsigned char buf[] = {0xD5,0x4C,0x01,0x00,0x01,0xCD,0x23,0xEF,0x45,0x67,0x89,0xAB,0x11,0x00,0xDC,0x10,0xFE,0x32,0xBA,0x98,0x76,0x54,0x00,0x11,0x40};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
		}while(0);
#endif
#if 0
		do{
			unsigned char buf[] = {0xE0,0x24,0x33,0x36,0x3F,0x3f,0x3f,0x3c,0x56,0x05,0x0c,0x0e,0x11,0x13,0x12,0x14,0x12,0x1e,0x24,0x33,0x36,0x3f,0x3f,0x3f,0x3c,0x56,0x05,0x0c,0x0e,0x11,0x13,0x12,0x14,0x12,0x1e};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);
#else
		do{
			unsigned char buf[] = {0xE0,
0x24,
0x33,
0x36,
0x3F,
0x3f,
0x3f,
0x3c,
0x56,
0x05,
0x0c,
0x0e,
0x11,
0x13,
0x12,
0x14,
0x12,
0x1e,
0x24,
0x33,
0x36,
0x3f,
0x3f,
0x3f,
0x3c,
0x56,
0x05,
0x0c,
0x0e,
0x11,
0x13,
0x12,
0x14,
0x12,
0x1e,};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
		}while(0);
#endif
#if 1
		do{
			unsigned char buf[] = {0xC1,
0x01,
0x00,
0x0A,
0x16,
0x1B,
0x24,
0x2C,
0x32,
0x3B,
0x43,
0x4B,
0x52,
0x5A,
0x60,
0x69,
0x70,
0x78,
0x80,
0x88,
0x8F,
0x97,
0xA0,
0xA8,
0xB0,
0xB9,
0xC2,
0xCA,
0xD2,
0xDA,
0xE3,
0xEA,
0xF4,
0xFA,
0xFF,
0x05,
0xEA,
0xC9,
0xB0,
0x02,
0x5F,
0xFD,
0x73,
0xC0,
0x00,
0x0A,
0x14,
0x19,
0x21,
0x29,
0x2E,
0x35,
0x3D,
0x44,
0x4B,
0x53,
0x58,
0x5F,
0x66,
0x6D,
0x74,
0x7B,
0x82,
0x88,
0x90,
0x98,
0x9F,
0xA7,
0xAF,
0xB6,
0xBE,
0xC6,
0xCD,
0xD4,
0xDD,
0xE3,
0xE9,
0x05,
0xEA,
0xC9,
0xB1,
0x02,
0x5F,
0xFD,
0x73,
0xC0,
0x00,
0x08,
0x11,
0x18,
0x21,
0x29,
0x2F,
0x37,
0x3F,
0x47,
0x4E,
0x56,
0x5C,
0x64,
0x6B,
0x73,
0x7A,
0x82,
0x89,
0x90,
0x99,
0xA1,
0xA8,
0xB1,
0xB9,
0xC0,
0xC9,
0xD0,
0xD7,
0xDF,
0xE7,
0xED,
0xF6,
0x05,
0xEA,
0xC9,
0xB1,
0x02,
0x5F,
0xFD,
0x73,
0xC0,
};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
		}while(0);
#endif
		otm1280a_write_1(0xe3, 0x01);
		do{
			unsigned char buf[] = {0xE5,
0x00,
0x00,
0x04,
0x04,
0x02,
0x00,
0x80,
0x20,
0x00,
0x20,
0x00,
0x00,
0x08,
0x06,
0x04,
0x00,
0x80,
0x0E,};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);

		do{
			unsigned char buf[] = {0xC7,0x00,0x20};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);

		do{
			unsigned char buf[] = {0xcc,0x01};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);

		do{
			unsigned char buf[] = {0xb6,0x2a};
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
		}while(0);
#ifdef LCD_CE_INTERFACE
		ce_on_off(ce_mode);
#endif
		otm1280a_write_1(0x36, 0x01);
		/* Set Display ON */
		otm1280a_write_0(0x29);
		mdelay(20);
	} else {
		printk("%s LG\n", __func__);
#if 1
		do{
			unsigned char buf[] = {0xE0,0x43,0x00,0x80,0x00,0x00};
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
		}while(0);	
		//		mdelay(1);
		//Display mode
		do{
			//unsigned char buf[] = {0xB5,0x2A,0x20,0x40,0x00,0x20};
			unsigned char buf[] = {0xB5,0x34,0x20,0x40,0x00,0x20};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);
		//GIP SDAC
		do{
			//unsigned char buf[] = {0xB1,0x7C,0x00,0x34,0x09,0x01,0x11,0x11,0x36,0x3E,0x26,0x26,0x57,0x12,0x01,0xE6};
			//unsigned char buf[] = {0xB6,0x04,0x34,0x0F,0x16,0x13};
			unsigned char buf[] = {0xB6,0x04,0x74,0x0F,0x16,0x13};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);
		//internal ocsillator setting
		do{
			unsigned char buf[] = {0xc0,0x01,0x08};
#if 1
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		do{
			unsigned char buf[] = {0xc1,0x00};
#if 1
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);

		//Power setting 
		do{
			//unsigned char buf[] = {0xC3,0x01,0x09,0x10,0x02,0x00,0x66,0x20,0x03,0x02};
			unsigned char buf[] = {0xC3,0x00,0x09,0x10,0x02,0x00,0x66,0x20,0x13,0x00};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);

		do{
			//unsigned char buf[] = {0xC4,0x22,0x24,0x12,0x18,0x59};
			unsigned char buf[] = {0xC4,0x23,0x24,0x17,0x17,0x59};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);

		//Gamma setting
		do{
			//		unsigned char buf[] = {0xD0,0x10,0x60,0x67,0x35,0x00,0x06,0x60,0x21,0x02};
			unsigned char buf[] = {0xD0,0x21,0x13,0x67,0x37,0x0c,0x06,0x62,0x23,0x03};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif

		}while(0);
		//		mdelay(1);
		do{
			//unsigned char buf[] = {0xD1,0x21,0x61,0x66,0x35,0x00,0x05,0x60,0x13,0x01};
			unsigned char buf[] = {0xD1,0x32,0x13,0x66,0x37,0x02,0x06,0x62,0x23,0x03};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);

		do{
			//unsigned char buf[] = {0xD2,0x10,0x60,0x67,0x40,0x1F,0x06,0x60,0x21,0x02};
			unsigned char buf[] = {0xD2,0x41,0x14,0x56,0x37,0x0c,0x06,0x62,0x23,0x03};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);
		do{
			//	unsigned char buf[] = {0xD3,0x21,0x61,0x66,0x40,0x1F,0x05,0x60,0x13,0x01};
			unsigned char buf[] = {0xD3,0x52,0x14,0x55,0x37,0x02,0x06,0x62,0x23,0x03};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);
		do{
			//	unsigned char buf[] = {0xD4,0x10,0x60,0x67,0x35,0x00,0x06,0x60,0x21,0x02};
			unsigned char buf[] = {0xD4,0x41,0x14,0x56,0x37,0x0c,0x06,0x62,0x23,0x03};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);
		do{
			//unsigned char buf[] = {0xD5,0x21,0x61,0x66,0x35,0x00,0x05,0x60,0x13,0x01};
			unsigned char buf[] = {0xD5,0x52,0x14,0x55,0x37,0x02,0x06,0x62,0x23,0x03};
#if 0
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//Gamma setting end
		//		mdelay(1);
		do{
			unsigned char buf[] = {0x36,0x0B };
#if 1
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
		//		mdelay(1);
		//		otm1280a_write_0(0x11);
		//		mdelay(5);
		do{
			//unsigned char buf[] = {0xF9,0x80,0x00};
			unsigned char buf[] = {0xF9,0x00};
#if 1
			ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
			if (ret == 0){
				printk("cmmd_write error\n");
				return ret;
			}
#else
			ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
			if (ret == DSIM_FALSE){
				printk("cmd_write error\n");
				return ret;
			}
#endif
		}while(0);
#endif

#ifdef LCD_CE_INTERFACE
			otm1280a_write_1(0x70, 0x00);

			do{
				unsigned char buf[] = {0x71,0x00,0x00, 0x01, 0x01};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x72,0x01,0x0e};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x73,0x34,0x52, 0x00};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);
#if 0
			do{
				unsigned char buf[] = {0x74,0x05,0x00, 0x06};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x75,0x03,0x00,0x07};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x76,0x07, 0x00 ,0x06};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);
#endif

			do{
				unsigned char buf[] = {0x77,0x3f,0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x78,0x40,0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x79,0x40,0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x7A,0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x7B,0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);

			do{
				unsigned char buf[] = {0x7C,0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#if 0
				ret = ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int) buf, sizeof(buf));
				if (ret == 0){
					printk("cmmd_write error\n");
					return ret;
				}
#else
				ret = lcd_init_cmd_write(ddi_pd->dsim_base, DCS_LONG_WR,  buf, sizeof(buf));
				if (ret == DSIM_FALSE){
					printk("cmd_write error\n");
					return ret;
				}
#endif
			}while(0);
			ce_on_off(ce_mode);
#endif
	}

	return DSIM_TRUE;
}

void lcd_panel_init(void)
{
	lcd_pannel_on();
}

static int k3_panel_init(void)
{
	lcd_panel_init();

	return 0;
}

static int otm1280a_set_link(void *pd, unsigned int dsim_base,
		unsigned char (*cmd_write) (unsigned int dsim_base, unsigned int data0,
			unsigned int data1, unsigned int data2),
		unsigned char (*cmd_read) (unsigned int dsim_base, unsigned int data0,
			unsigned int data1, unsigned int data2))
{
	struct mipi_ddi_platform_data *temp_pd = NULL;

	temp_pd = (struct mipi_ddi_platform_data *) pd;
	if (temp_pd == NULL) {
		printk(KERN_ERR "mipi_ddi_platform_data is null.\n");
		temp_pd = malloc(sizeof(struct mipi_ddi_platform_data));
		//return -1;
	}

	ddi_pd = temp_pd;

	ddi_pd->dsim_base = dsim_base;

	if (cmd_write)
		ddi_pd->cmd_write = cmd_write;
	else
		printk(KERN_WARNING "cmd_write function is null.\n");

	if (cmd_read)
		ddi_pd->cmd_read = cmd_read;
	else
		printk(KERN_WARNING "cmd_read function is null.\n");

	return 0;
}

static int otm1280a_probe(struct device *dev)
{
	return 0;
}



struct mipi_lcd_driver otm1280a_mipi_driver = {
	.name = "k3_mipi_lcd",
	.init = k3_panel_init,
	.display_on = otm1280a_display_on,
	.set_link = otm1280a_set_link,
	.probe = otm1280a_probe,
	.display_off = otm1280a_display_off,
};


