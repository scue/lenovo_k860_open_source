/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * S5PC220 - LCD Driver for U-Boot
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/types.h>
#include "dsim.h"

#include <font_CN16x32.h>

#define LCD_BGCOLOR		0x1428A0

extern int lcd_id;
static unsigned int gFgColor = 0xFF;
static unsigned int gLeftColor = LCD_BGCOLOR;

#define Inp32(_addr)		readl(_addr)
#define Outp32(addr, data)	(*(volatile u32 *)(addr) = (data))
#define Delay(_a)		udelay(_a*1000)
#define CFG_LCD_FBUFFER 0x47000000


#define LCD_WIDTH		720
#define LCD_HEIGHT		1280


#define S5PV310_PA_LCD0 (0x11C00000)

void LCD_Initialize(void)
{
    u32 uFbAddr = CFG_LCD_FBUFFER;
    u32 i,uTmpReg;
    u32* pBuffer = (u32*)uFbAddr;
    Outp32(0x1003c234, (Inp32(0x1003c234)&(~(0xf<<0)))|0x6);
    Outp32(0x1003c534, (Inp32(0x1003c534)&(~(0xf<<0)))|0x0);

    Outp32(S5PV310_PA_LCD0+0x4, 0x00);
    Outp32(S5PV310_PA_LCD0+0x8, 0);
    Outp32(S5PV310_PA_LCD0+0xc, 0);
	if (lcd_id == LCD_BOE) {
		Outp32(S5PV310_PA_LCD0+0x10, (0x0 << 24) | (0x0a << 16) | (0x0a << 8) | (0x01 << 0));
		Outp32(S5PV310_PA_LCD0+0x14, (0x3a << 16) | (0x1f << 8) | (0x08 << 0));
		Outp32(S5PV310_PA_LCD0+0x170, 0x0);
		Outp32(S5PV310_PA_LCD0+0x18, (0x2cf <<0)|(0x4ff << 11));
	} else {
		Outp32(S5PV310_PA_LCD0+0x10, (0x0 << 24) | (11 << 16) | (7 << 8) | (0x01 << 0));
		Outp32(S5PV310_PA_LCD0+0x14, (0x0 << 24) | (111 << 16) | (11 << 8) | (3 << 0));
		Outp32(S5PV310_PA_LCD0+0x170, 0x0);
		Outp32(S5PV310_PA_LCD0+0x18, (0x2cf <<0)|(0x4ff << 11));
	}
    Outp32(S5PV310_PA_LCD0+0x130, 0x1); //0x20->0x1
    Outp32(S5PV310_PA_LCD0+0x20, 0x0);
    Outp32(S5PV310_PA_LCD0+0x24, 0x0);
    Outp32(S5PV310_PA_LCD0+0x28, 0x8077);//
    Outp32(S5PV310_PA_LCD0+0x2c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x30, 0x0);
    Outp32(S5PV310_PA_LCD0+0x34, 0x4);
    Outp32(S5PV310_PA_LCD0+0x180, 0x0);
    Outp32(S5PV310_PA_LCD0+0x184, 0x0);
    Outp32(S5PV310_PA_LCD0+0x188, 0x0);
    Outp32(S5PV310_PA_LCD0+0x18c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x190, 0x0);
    Outp32(S5PV310_PA_LCD0+0x140, 0x0);
    Outp32(S5PV310_PA_LCD0+0x148, 0x0);
    Outp32(S5PV310_PA_LCD0+0x150, 0x0);
    Outp32(S5PV310_PA_LCD0+0x158, 0x0);
    Outp32(S5PV310_PA_LCD0+0x58, 0x0);
    Outp32(S5PV310_PA_LCD0+0x208, 0x0);
    Outp32(S5PV310_PA_LCD0+0x20c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x68, 0x0);
    Outp32(S5PV310_PA_LCD0+0x210, 0x0);
    Outp32(S5PV310_PA_LCD0+0x214, 0x0);
    Outp32(S5PV310_PA_LCD0+0x78, 0x0);
    Outp32(S5PV310_PA_LCD0+0x218, 0x0);
    Outp32(S5PV310_PA_LCD0+0x21c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x88, 0x0);
    Outp32(S5PV310_PA_LCD0+0x220, 0x0);
    Outp32(S5PV310_PA_LCD0+0x224, 0x0);
    Outp32(S5PV310_PA_LCD0+0x260, 0x1);
    Outp32(S5PV310_PA_LCD0+0x34, 0x0);

    Outp32(S5PV310_PA_LCD0+0xb4, uFbAddr + 0x0);

    Outp32(S5PV310_PA_LCD0+0xb0, uFbAddr + 0x0); 
    Outp32(S5PV310_PA_LCD0+0x108, 0x780);
    Outp32(S5PV310_PA_LCD0+0x28, 0x8000);
    Outp32(S5PV310_PA_LCD0+0x28, 0x802c);

    Outp32(S5PV310_PA_LCD0+0x60, 0x0);
    Outp32(S5PV310_PA_LCD0+0x64, 0x167cff);
    Outp32(S5PV310_PA_LCD0+0x200, 0x1000100);
    Outp32(S5PV310_PA_LCD0+0x204, 0x1000100);
    Outp32(S5PV310_PA_LCD0+0x28, 0x802d);
    Outp32(S5PV310_PA_LCD0+0x34, 0x4);
    Outp32(S5PV310_PA_LCD0+0x34, 0x4);
    Outp32(S5PV310_PA_LCD0+0x34, 0x404);
    Outp32(S5PV310_PA_LCD0+0x28, 0x802d);
    Outp32(S5PV310_PA_LCD0, 0x40010303);
}


static void FIMD_reset(void)
{
    u32 uFbAddr = CFG_LCD_FBUFFER;

    Outp32(S5PV310_PA_LCD0+0x4, 0);
    Outp32(S5PV310_PA_LCD0+0x10, 0);
    Outp32(S5PV310_PA_LCD0+0x14, 0);
    Outp32(S5PV310_PA_LCD0+0x170, 0x0);
    Outp32(S5PV310_PA_LCD0+0x18, 0);
    Outp32(S5PV310_PA_LCD0, 0x0);
    Outp32(S5PV310_PA_LCD0, 0);
    Outp32(S5PV310_PA_LCD0+0x130, 0);
    Outp32(S5PV310_PA_LCD0+0x20, 0x0);
    Outp32(S5PV310_PA_LCD0+0x24, 0x0);
    Outp32(S5PV310_PA_LCD0+0x28, 0x0);
    Outp32(S5PV310_PA_LCD0+0x2c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x30, 0x0);
    Outp32(S5PV310_PA_LCD0+0x34, 0x0);
    Outp32(S5PV310_PA_LCD0+0x180, 0x0);
    Outp32(S5PV310_PA_LCD0+0x184, 0x0);
    Outp32(S5PV310_PA_LCD0+0x188, 0x0);
    Outp32(S5PV310_PA_LCD0+0x18c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x190, 0x0);
    Outp32(S5PV310_PA_LCD0+0x140, 0x0);
    Outp32(S5PV310_PA_LCD0+0x148, 0x0);
    Outp32(S5PV310_PA_LCD0+0x150, 0x0);
    Outp32(S5PV310_PA_LCD0+0x158, 0x0);
    Outp32(S5PV310_PA_LCD0+0x58, 0x0);
    Outp32(S5PV310_PA_LCD0+0x208, 0x0);
    Outp32(S5PV310_PA_LCD0+0x20c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x68, 0x0);
    Outp32(S5PV310_PA_LCD0+0x210, 0x0);
    Outp32(S5PV310_PA_LCD0+0x214, 0x0);
    Outp32(S5PV310_PA_LCD0+0x78, 0x0);
    Outp32(S5PV310_PA_LCD0+0x218, 0x0);
    Outp32(S5PV310_PA_LCD0+0x21c, 0x0);
    Outp32(S5PV310_PA_LCD0+0x88, 0x0);
    Outp32(S5PV310_PA_LCD0+0x220, 0x0);
    Outp32(S5PV310_PA_LCD0+0x224, 0x0);
    Outp32(S5PV310_PA_LCD0+0x260, 0x1);
    Outp32(S5PV310_PA_LCD0+0x34, 0x0);
    Outp32(S5PV310_PA_LCD0+0xa4, 0x0);
    Outp32(S5PV310_PA_LCD0+0xd4, 0);
    Outp32(S5PV310_PA_LCD0+0xa0, 0x0);
    Outp32(S5PV310_PA_LCD0+0xd0, 0);
    Outp32(S5PV310_PA_LCD0+0x20a0, 0x0);
    Outp32(S5PV310_PA_LCD0+0x20d0, 0);
    Outp32(S5PV310_PA_LCD0+0x100, 0);
    Outp32(S5PV310_PA_LCD0+0x20, 0);
    Outp32(S5PV310_PA_LCD0+0x20, 0);
    Outp32(S5PV310_PA_LCD0+0x40, 0x0);
    Outp32(S5PV310_PA_LCD0+0x44, 0);
    Outp32(S5PV310_PA_LCD0+0x200, 0);
    Outp32(S5PV310_PA_LCD0+0x204, 0);
    Outp32(S5PV310_PA_LCD0+0x34, 0);
    Outp32(S5PV310_PA_LCD0+0x20, 0);
    Outp32(S5PV310_PA_LCD0+0x34, 0);
    Outp32(S5PV310_PA_LCD0+0x34, 0);
    Outp32(S5PV310_PA_LCD0+0x34, 0);
    Outp32(S5PV310_PA_LCD0+0x20, 0);
    Outp32(S5PV310_PA_LCD0+0x34, 0);
    Outp32(S5PV310_PA_LCD0+0x34, 0);

    Outp32(S5PV310_PA_LCD0, 0);


}
/*
*/
#define CALC_MAX77686_LDO_VOL(x)      ( (x<800000 || x>3950000) ? 0 : ((x-800000)/50000) )

int g_lcd_on = 0;
void LCD_turnon(void)
{
    if(g_lcd_on == 1)
        return;

    g_lcd_on = 1;
    FIMD_reset();
    LCD_Initialize();
    s5p_dsim_probe();
//    lcd_backlight_init();
    //	LCD_setfgcolor(0xff);
    //	lcd_draw_bgcolor();
    //	lcd_draw_string(0, 0, "LCD TEST\n", 0xff0000);
}

void LCD_turnoff(void)
{
    LCD_setfgcolor(0x0);
    lcd_draw_bgcolor();
    return;
    FIMD_reset();
}

void LCD_setfgcolor(unsigned int color)
{
    gFgColor = color;
}

void LCD_setleftcolor(unsigned int color)
{
    gLeftColor = color;
}

void LCD_setprogress(int percentage)
{

    u32 i, j;
    u32* pBuffer = (u32*)CFG_LCD_FBUFFER;

    for (i=0; i < (LCD_HEIGHT/100)*percentage; i++)
    {
        for (j=0; j < LCD_WIDTH; j++)
        {
            *pBuffer++ = gFgColor;
        }
    }

    for (; i < LCD_HEIGHT; i++)
    {
        for (j=0; j < (LCD_WIDTH >> 5); j++)
        {
            *pBuffer++ = gLeftColor;
        }
        for (; j < LCD_WIDTH; j++)
        {
            *pBuffer++ = LCD_BGCOLOR;
        }
    }

}

//fangcg:  lcd is 4 bytes per pixel, but we only use 24 colors.
#define FONT_HEIGHT  42
#define FONT_WIDTH   24
#define LCD_LINE_LENGTH  LCD_WIDTH
#define ROWVALUE  (LCD_HEIGHT / FONT_HEIGHT)
#define COLVALUE   (LCD_WIDTH / FONT_WIDTH)

struct bFrameBuffer
{
    u32 pixel[LCD_HEIGHT][LCD_WIDTH];
}*g_pFrameBuf;

static char strbuffer[ROWVALUE][COLVALUE];

void lcd_draw_bgcolor()
{
    u32 i, j;
    u32 *pBuffer = (u32 *)CFG_LCD_FBUFFER;

    for (i = 0; i < LCD_HEIGHT; i++)
    {
        for (j = 0; j < LCD_WIDTH; j++)
        {
            *pBuffer++ = gFgColor;
        }
    }
}

void lcd_draw_rect(u32 xstart, u32 ystart, u32 rect_len, u32 rect_high, u32 color)
{
    u32 i,j;
    u32 *pBuffer = (u32 *)CFG_LCD_FBUFFER + ystart * LCD_WIDTH + xstart;

    for (j = ystart; j < ystart + rect_high; j++)
    {
        u32 *d = pBuffer;
        for (i = xstart; i < xstart + rect_len; i++)
        {
            *d++ = color;
        }
        pBuffer += LCD_LINE_LENGTH;
    }
}

void lcd_draw_char(u32 row, u32 col, uchar c, u32 color)
{
    uchar *font;
    u32 *pix;
    u32 f;
    int r;
    unsigned int offset = 0;
    u32 *pFramebuf = (u32 *)CFG_LCD_FBUFFER;

    if ((row > ROWVALUE) || (col >COLVALUE))
    {
        return -1;
    };

    if (c < 32 || c >= 127)
	return -1;

    g_pFrameBuf = (struct bFrameBuffer *)CFG_LCD_FBUFFER;
    pix = &g_pFrameBuf->pixel[((row * FONT_HEIGHT))][col * FONT_WIDTH];
    font = &ac16x32CN[((c - 32) * (FONT_HEIGHT * 3))];
    for (r = 0; r < FONT_HEIGHT; r++)
    {
        f = *font++;

        offset = (row * FONT_HEIGHT + r) * LCD_WIDTH + (col * FONT_WIDTH);
        *pix++ = (((f>>7) & 0x1)? color: pFramebuf[offset + 0]);
        *pix++ = (((f>>6) & 0x1)? color: pFramebuf[offset + 1]);
        *pix++ = (((f>>5) & 0x1)? color: pFramebuf[offset + 2]);
        *pix++ = (((f>>4) & 0x1)? color: pFramebuf[offset + 3]);
        *pix++ = (((f>>3) & 0x1)? color: pFramebuf[offset + 4]);
        *pix++ = (((f>>2) & 0x1)? color: pFramebuf[offset + 5]);
        *pix++ = (((f>>1) & 0x1)? color: pFramebuf[offset + 6]);
        *pix++ = (((f>>0) & 0x1)? color: pFramebuf[offset + 7]);

        f = *font++;
        *pix++ = (((f>>7) & 0x1)? color: pFramebuf[offset + 8]);
        *pix++ = (((f>>6) & 0x1)? color: pFramebuf[offset + 9]);
        *pix++ = (((f>>5) & 0x1)? color: pFramebuf[offset + 10]);
        *pix++ = (((f>>4) & 0x1)? color: pFramebuf[offset + 11]);
        *pix++ = (((f>>3) & 0x1)? color: pFramebuf[offset + 12]);
        *pix++ = (((f>>2) & 0x1)? color: pFramebuf[offset + 13]);
        *pix++ = (((f>>1) & 0x1)? color: pFramebuf[offset + 14]);
        *pix++ = (((f>>0) & 0x1)? color: pFramebuf[offset + 15]);

        f = *font++;
        *pix++ = (((f>>7) & 0x1)? color: pFramebuf[offset + 16]);
        *pix++ = (((f>>6) & 0x1)? color: pFramebuf[offset + 17]);
        *pix++ = (((f>>5) & 0x1)? color: pFramebuf[offset + 18]);
        *pix++ = (((f>>4) & 0x1)? color: pFramebuf[offset + 19]);
        *pix++ = (((f>>3) & 0x1)? color: pFramebuf[offset + 20]);
        *pix++ = (((f>>2) & 0x1)? color: pFramebuf[offset + 21]);
        *pix++ = (((f>>1) & 0x1)? color: pFramebuf[offset + 22]);
        *pix++ = (((f>>0) & 0x1)? color: pFramebuf[offset + 23]);

        pix += (LCD_WIDTH - 24);
    }

    return 0;
}

void lcd_draw_string(u32 row, u32 col, char *c, u32 color)
{
    u32  i,j, x,nrow,ncol;

    nrow = row;
    ncol  = col;

    if (nrow > ROWVALUE ||ncol>COLVALUE)
    {
        return;
    }

    if (c[0] == 0) return;

    if (c[1] == 0)
    {
        x=1;
    }
    else
        x = strlen(c);

    for (i = 0; i < x; i++)
    {
        if (c[i] == '\n')
        {
            for (j = ncol; j < COLVALUE; j++)
            {
                strbuffer[nrow][ncol] = ' ';
                lcd_draw_char(nrow, ncol, strbuffer[nrow][ncol], color);
            }
            nrow += 1;
            if (nrow > ROWVALUE)
            {
                //scroll();
                nrow = ROWVALUE;
            }
            ncol = 0;
        }
        else
        {
            strbuffer[nrow][ncol] = c[i];
            lcd_draw_char(nrow, ncol, strbuffer[nrow][ncol], color);
            ncol += 1;
            if (ncol > COLVALUE-1)
            {
                nrow += 1;
                if (nrow > ROWVALUE)
                {
                    //scroll();
                    nrow = ROWVALUE;
                }
                ncol = 0;
            }
        }
    }
}

u32 get_lcd_framebuf(void)
{
	return (u32*)CFG_LCD_FBUFFER;
}

int get_lcd_width(void)
{
	return LCD_WIDTH;
}

int get_lcd_height(void)
{
	return LCD_HEIGHT;
}
