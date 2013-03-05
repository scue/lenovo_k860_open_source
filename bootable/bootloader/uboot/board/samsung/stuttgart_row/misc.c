/*
*
* File: $(BOARD)/misc.c
*
*/

#include <common.h>
#include <command.h>
#include <malloc.h>

#include <asm/io.h>

#define ELFIN_GPIO_BASE			0x11000000
#define GPX0CON_OFFSET  		0x0c00
#define GPX0DAT_OFFSET  		0x0c04
#define GPX0PUD_OFFSET  		0x0c08

#define GPX1CON_OFFSET  		0x0c20
#define GPX1DAT_OFFSET  		0x0c24
#define GPX1PUD_OFFSET  		0x0c28

#define GPX2CON_OFFSET  		0x0c40
#define GPX2DAT_OFFSET  		0x0c44
#define GPX2PUD_OFFSET  		0x0c48

#define GPX3CON_OFFSET  		0x0c60
#define GPX3DAT_OFFSET  		0x0c64
#define GPX3PUD_OFFSET  		0x0c68

#define GPX0CON 		(ELFIN_GPIO_BASE + GPX0CON_OFFSET)
#define GPX0DAT 		(ELFIN_GPIO_BASE + GPX0DAT_OFFSET)
#define GPX0PUD 		(ELFIN_GPIO_BASE + GPX0PUD_OFFSET)

#define GPX1CON 		(ELFIN_GPIO_BASE + GPX1CON_OFFSET)
#define GPX1DAT 		(ELFIN_GPIO_BASE + GPX1DAT_OFFSET)
#define GPX1PUD 		(ELFIN_GPIO_BASE + GPX1PUD_OFFSET)

#define GPX2CON 		(ELFIN_GPIO_BASE + GPX2CON_OFFSET)
#define GPX2DAT 		(ELFIN_GPIO_BASE + GPX2DAT_OFFSET)
#define GPX2PUD 		(ELFIN_GPIO_BASE + GPX2PUD_OFFSET)

#define GPX3CON 		(ELFIN_GPIO_BASE + GPX3CON_OFFSET)
#define GPX3DAT 		(ELFIN_GPIO_BASE + GPX3DAT_OFFSET)
#define GPX3PUD 		(ELFIN_GPIO_BASE + GPX3PUD_OFFSET)

#define GPF0DAT 		0x11400184

int g_update_flag = REBOOTFLAG_NONE;
int g_factory_mode = 0;
int g_boot_mode = 0;
int enter_mode_a(void);
int check_boot_mode(int recovery_mode)
{
	unsigned int reg;
	unsigned int reboot_flag;

	keylight(1);

	// check GPF0_4
	reg = readl(GPF0DAT);
	if(reg&0x10)
		g_factory_mode = 1;

	// 1. check reboot flag
	reboot_flag = REBOOTFLAG;
	REBOOTFLAG = REBOOTFLAG_NONE;
	printf("[Lenovo]REBOOTFLAG = 0x%x\n", reboot_flag);
	if(reboot_flag == REBOOTFLAG_SDUPDATE_OTHER)
	{
		switch_audio_jack_func(1);
		printf("[Lenovo]Bootmode: SD UPDATE OTHER\n");
		g_update_flag = REBOOTFLAG_SDUPDATE_OTHER;
		run_command("sdfuse", 0);
		return 0;
	}
	/* didn't enter cp update mode if recovery mode is enable */
	else if ((reboot_flag == REBOOTFLAG_CPUPDATE) && (recovery_mode == 0))
	{
		if(g_factory_mode)
			clear_cp_update_flag();
		printf("[Lenovo]Bootmode: CP Update\n");
		setenv("bootargs", CONFIG_BOOTARGS);
		run_command("setenv bootargs ${bootargs} cp_update=1", 0);
		return 0;
	}

	// 3. check GPF0_4
	if(g_factory_mode)
	{
		switch_audio_jack_func(1);
		set_audio_jack_flag(1);
		printf("[Lenovo]Bootmode: SDUPDATE\n");
		g_update_flag = REBOOTFLAG_SDUPDATE_UBOOT;
		run_command("sdfuse", 0);
		return 0;
	}

	// 4. check keypad
	reg = readl(GPX1CON);
	reg = 0x555f;
	writel(reg, GPX1PUD);  	// Config GPX1_0/1 pull up
	udelay(1);

	reg = readl(GPX1DAT);
	if(0 == (reg&0x1)){
		printf("[Lenovo]Bootmode: Mode A\n");
		//setenv("bootargs", CONFIG_BOOTARGS);
		enter_mode_a();
	}
	else if(0 == (reg&0x2)){
		printf("[Lenovo]Bootmode: Mode B\n");
		setenv("bootargs", CONFIG_TMD_BOOTARGS);
		g_boot_mode = 1;
	}
	else{
		printf("[Lenovo]Bootmode: Normal Mode\n");
		setenv("bootargs", CONFIG_BOOTARGS);
	}

	// 5. check GPX2_1
	reg = readl(GPX2PUD);	// Config GPX2_1 pull up
	reg |= (0x3)<<2;
	writel(reg, GPX2PUD);
	udelay(1);

	reg = readl(GPX2DAT);
	if(0 == (reg&0x2))
	{
		printf("[Lenovo]Bootmode: RF Test\n");
		switch_audio_jack_func(1);
		run_command("modem -x", 0);
		while(1)
			udelay(10*1000*1000);
	}

	// 2. check cp update flag
	/* didn't enter cp update mode if recovery mode is enable */
	if(check_cp_update_flag() && (recovery_mode == 0))
	{
		//clear_cp_update_flag();
		printf("[Lenovo]Bootmode: CP Update\n");
		setenv("bootargs", CONFIG_BOOTARGS);
		run_command("setenv bootargs ${bootargs} cp_update=1", 0);
		return 0;
	}

	return 0;
}

void keylight(int on)
{
	__REG(GPX0CON) = (__REG(GPX0CON) & ~(0xf<<24)) | (0x1<<24);
	__REG(GPX0DAT) = (__REG(GPX0DAT) & 0xbf) | (on & 1)<<6;
}

extern void LCD_turnon(void);
extern void LCD_turnoff(void);
extern void LCD_setfgcolor(unsigned int color);
extern void lcd_draw_bgcolor();
extern void lcd_draw_string(u32 row, u32 col, char *c, u32 color);

#define COLOR_RED 		0xff0000
#define COLOR_GREEN 	0x00ff00
#define COLOR_BLUE 		0x0000ff

void display_logo(void)
{
	lcd_draw_logo();
	LCD_turnon();
//	LCD_setfgcolor(0x0);
//	lcd_draw_bgcolor();
}

int handle_rf_test(void)
{
	switch_audio_jack_func(1);
	lcd_draw_string(2, 2, "MODE_AMT ALIGNED ...", 0xFF00);
	run_command("modem -x", 0);
	while(1)
		udelay(10*1000*1000);
	return 0;
}

int handle_sd_update(void)
{
	switch_audio_jack_func(1);
	g_update_flag = REBOOTFLAG_SDUPDATE_UBOOT;
	run_command("sdfuse", 0);
	return 0;
}

int handle_usb_update(void)
{
	switch_audio_jack_func(1);
	run_command("fastboot", 0);
	return 0;
}

int handle_debug_on(void)
{
	switch_audio_jack_func(1);
	set_audio_jack_flag(0);
	printf("[Lenovo]Set AP Debug On\n");
	run_command("reset", 0);
	return 0;
}

int handle_debug_off(void)
{
	switch_audio_jack_func(1);
	set_audio_jack_flag(1);
	printf("[Lenovo]Set AP Debug Off\n");
	run_command("reset", 0);
	return 0;
}

int handle_reboot(void)
{
	switch_audio_jack_func(1);
	run_command("reset", 0);
	return 0;
}

struct TESTARRAY
{
    char name[32];
    int (*handler)(void);
};

struct TESTARRAY MainMenuArry[] = {
	{"RF Test",				handle_rf_test},
	{"SD Update",			handle_sd_update},
	{"USB Update",		handle_usb_update},
	{"Recovery",			NULL},
	{"AP Debug On",		handle_debug_on},
	{"AP Debug Off",	handle_debug_off},
	{"Reboot",				handle_reboot},
};

int g_cur_selected = 0;
int g_item_nums = sizeof(MainMenuArry)/sizeof(MainMenuArry[0]);
void draw_item(int index, int color)
{
	char str[4];
	memset(str, 0, 4);

	if (index < 10)
		sprintf(str, "%d.", index);
	else
		sprintf(str, "%d", index);

	lcd_draw_string(index + 4, 3, str, color);

	lcd_draw_string(index + 4, 5, MainMenuArry[index].name, color);
}

void draw_all_item(int len)
{
	int i = 0;

//	clr_screen();
	for (i = 0; i < len; i++)
	{
		if (i == g_cur_selected)
			draw_item(i, COLOR_RED);
		else
			draw_item(i, COLOR_GREEN);
	}

	return;
}

#define KEY_VOLUMEDOWN		114
#define KEY_VOLUMEUP				115
#define KEY_POWER						116
int enter_mode_a(void)
{
	unsigned int reg;
	int key;

	LCD_turnon();
	LCD_setfgcolor(0x0);
	lcd_draw_bgcolor();

	lcd_draw_string(2, 1, "MODE  A", COLOR_GREEN);
	draw_all_item(g_item_nums);

	// Config GPX3_1 PowerOn key detect gpio
	reg = readl(GPX3CON);		// Config GPX3_1 as input
	reg &= ~(0xf<<4);
	writel(reg, GPX3CON);
	udelay(1);

	reg = readl(GPX3PUD);		// Config GPX3_1 pull-up
	reg |= 0x0c;
	writel(reg, GPX3PUD);
	udelay(1);

	while(1)
	{
		udelay(100*1000);

		// key detect
		reg = readl(GPX1DAT);
		if(0 == (reg&0x1)){							// KEY_VOLUMEUP
			printf("VOLUME_UP pressed\n");
			key = KEY_VOLUMEUP;
		} else if (0 == (reg&0x2)){				// KEY_VOLUMEDOWN
			printf("VOLUME_PU pressed\n");
			key = KEY_VOLUMEDOWN;
		} else {
			// Power key detect
			reg = readl(GPX3DAT);
			if(0 == (reg&0x2)){						// KEY_POWER
				printf("KEY_POWER pressed\n");
				key = KEY_POWER;
			} else {
				continue;
			}
		}

		udelay(200*1000);
		if(key == KEY_VOLUMEUP)
		{
			g_cur_selected -= 1;
			if(g_cur_selected<0)
				g_cur_selected = g_item_nums-1;
		}
		else  if(key == KEY_VOLUMEDOWN)
		{
			g_cur_selected += 1;
			if(g_cur_selected == g_item_nums)
				g_cur_selected = 0;
		}
		else if(key == KEY_POWER)
		{
			if(MainMenuArry[g_cur_selected].handler != NULL)
			{
				LCD_setfgcolor(0x0);
				lcd_draw_bgcolor();
				printf("[Lenovo]Enter %s\n", MainMenuArry[g_cur_selected].name);
				MainMenuArry[g_cur_selected].handler();
				break;
			}
		}

		draw_all_item(g_item_nums);
	}

	return 0;
}


// end of file
