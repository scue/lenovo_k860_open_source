/*
*
* File: flags.c, moved from misc.c
*
*/

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <mmc.h>

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>
#include <asm/arch/movi_partition.h>

#define PARAM_BLKSIZE  	512
/* #define DEBUG_RECOVERY	1 */
/* #define DEBUG_PARAM		1 */

/* add your own flag between PARAM_WSINFO and PARAM_MAX */
typedef enum {
	PARAM_CPFLAG = 0,
	PARAM_HSFLAG,
	PARAM_SDFLAG,
	PARAM_RECOVERYFLAG,
	PARAM_RECOVERYFLAG_END = PARAM_RECOVERYFLAG + 2,
	PARAM_PSCAL,
	PARAM_WSINFO,
	PARAM_LOGO,			  // LOGO size = 40k
	PARAM_LOGO_END = PARAM_LOGO+80,
	PARAM_MAX
}param_data_t;

extern int get_raw_area_info(char *name, unsigned int *start, unsigned int *size);
unsigned int get_inform4(void);

/* read parameter partion */
static int param_block_read(param_data_t blk, void *buf, int len)
{
	int ret;
	unsigned int start, count;
	struct mmc *mmc;
	unsigned long offset;
	unsigned char rbuf[PARAM_BLKSIZE];

	ret = get_raw_area_info("parameter", &start, &count);
	if(ret < 0)
	{
		printf("Fail to get parameter partition\n");
		return -1;
	}
#ifdef DEBUG_PARAM
	printf("Parameter: start = %d len = %d(~%dKB)\n", start, count, count>>10);
#endif

	offset = start/PARAM_BLKSIZE + blk;

	mmc = find_mmc_device(0);

	ret = mmc->block_dev.block_read(0, offset, 1, rbuf);
	if(ret!=1)
	{
		printf("Fail to read parameter partition, block %d\n", blk);
		return -1;
	}

	memcpy(buf, rbuf, len);

	return 0;
}

static int param_block_write(param_data_t blk, void *buf, int len)
{
	int ret;
	unsigned int start, count;
	struct mmc *mmc;
	unsigned long offset;
	unsigned char wbuf[PARAM_BLKSIZE];

	memset(wbuf, 0, PARAM_BLKSIZE);
	ret = get_raw_area_info("parameter", &start, &count);
	if(ret < 0)
	{
		printf("Fail to get parameter partition\n");
		return -1;
	}
#ifdef DEBUG_PARAM
	printf("Parameter: start = %d len = %d(~%dKB)\n", start, count, count>>10);
#endif

	offset = start/PARAM_BLKSIZE + blk;

	mmc = find_mmc_device(0);

	memcpy(wbuf, buf, len);
	ret = mmc->block_dev.block_write(0, offset, 1, wbuf);
	if(ret!=1)
	{
		printf("Fail to write parameter partition, block %d\n", blk);
		return -1;
	}

	return 0;
}

/* cp_update_flag = 0x11223344 */
int write_cp_update_flag(void)
{
	int ret;
	int flag = 0x11223344;

	printf("[Lenovo]Write cp update flag: 0x11223344\n");
	ret = param_block_write(PARAM_CPFLAG, &flag, 4);
	if(ret < 0)
		printf("[Lenovo]Fail to write cp update flag\n");

	return 0;
}

int check_cp_update_flag(void)
{
	int ret, flag;

	ret = param_block_read(PARAM_CPFLAG, &flag, 4);
	if(ret < 0)
		printf("[Lenovo]Fail to read cp update flag\n");

	printf("[Lenovo]CP FLAG: 0x%08x\n", flag);

	if(0x11223344 == flag)
		return 1;

	return 0;
}

int clear_cp_update_flag(void)
{
	int ret, flag = 0;

	printf("[Lenovo]Clear cp update flag\n");
	ret = param_block_write(PARAM_CPFLAG, &flag, 4);
	if(ret<0)
		printf("[Lenovo]Fail to clear cp update flag\n");

	return 0;
}

#define GPF2CON 		(0x114001C0)
#define GPF2DAT 		(0x114001C4)
#define GPF2PUD 		(0x114001C8)

//
// 0: Headset
// 1: AP Uart
//
void switch_audio_jack_func(int lvl)
{
	unsigned int reg;

	reg = readl(GPF2CON);		// Config GPF2_7 as output 
	reg &= ~(0xf<<28);
	reg |= 0x1<<28;
	writel(reg, GPF2CON);

	reg = readl(GPF2DAT);			// 
	reg &= ~(0x1<<7);
	if(lvl)
		reg |= 0x1<<7;
	writel(reg, GPF2DAT);
}

//
// 0: uart flag
// 1: headset flag
//
void set_audio_jack_flag(int f)
{
	int ret;
	int flag = 0xFFFFFFFF;

	if(f == 0)
		flag = 0xAABBCCDD;

	ret = param_block_write(PARAM_HSFLAG, &flag, sizeof(int));
	if(ret != 0)
		printf("[Lenovo]Fail to write debug2hs flag\n");

	ret = param_block_read(PARAM_HSFLAG, &flag, sizeof(int));
	printf("[Lenovo]debug2hs flag = 0x%x\n", flag);
}

extern int display_banner (void);
extern int print_cpuinfo (void);
extern int display_dram_config (void);

extern unsigned int OmPin;
void display_boot_info(void)
{
	char bl1_version[9] = {0};

	printf(" \n");
	display_banner();
	print_cpuinfo();

	// Display dram info
	display_dram_config();

	// Display BL1 version
#ifdef CONFIG_TRUSTZONE
	printf("BL1 version: N/A (TrustZone Enabled BSP)\n");
#else
	strncpy(&bl1_version[0], (char *)0x02022fc8, 8);
	printf("BL1 version: %s\n", &bl1_version[0]);
#endif

	// Display boot mode
	printf("\n\nChecking Boot Mode ...");
	if(OmPin == BOOT_ONENAND) {
		printf(" OneNand\n");
	} else if (OmPin == BOOT_NAND) {
		printf(" NAND\n");
	} else if (OmPin == BOOT_MMCSD) {
		printf(" SDMMC\n");
	} else if (OmPin == BOOT_EMMC) {
		printf(" EMMC4.3\n");
	} else if (OmPin == BOOT_EMMC_4_4) {
		printf(" EMMC4.41\n");
	}
	printf("\n");

}

/*
* check audio jack function flag
* default is headset
*/
int check_audio_jack_flag(void)
{
	int ret, flag;

	ret = param_block_read(PARAM_HSFLAG, &flag, sizeof(int));
	if(ret != 0){
		printf("[Lenovo] Fail to read debug2hs flag\n");
		return -1;
	}

	if ((0xAABBCCDD == flag) ||check_cp_update_flag()){
		// audio jack as uart port
		switch_audio_jack_func(1);
		display_boot_info();
		run_command("setenv bootargs ${bootargs} debug2hs=0", 0);
	} else {
		// audio jack as headset function
		switch_audio_jack_func(0);
		run_command("setenv bootargs ${bootargs} debug2hs=1", 0);
	}

	printf("[Lenovo] debug2hs = 0x%x\n", flag);
	return ret;
}

int check_pscal(void)
{
	int ret;
	char rbuf[32];
	int low;

	ret = param_block_read(PARAM_PSCAL, rbuf, 32);
	if(ret != 0){
		printf("[Lenovo] Fail to read debug2hs flag\n");
		return -1;
	}

	printf("[Lenovo] rbuf = %s\n", rbuf);
	low = (rbuf[0]-'0')*100 + (rbuf[1]-'0')*10 + (rbuf[2]-'0');

	if(low == 300)
		run_command("setenv bootargs ${bootargs} pscal=1", 0);
	else
		run_command("setenv bootargs ${bootargs} pscal=0", 0);

	return ret;
}

struct bootloader_message {
	char command[32];
	char status[32];
	char recovery[1024];
};

int get_bootloader_message(void *buf, int len)
{
	int ret = 0, i = 0;

	while (len > 0) {
		if (len >= PARAM_BLKSIZE)
			ret = param_block_read(PARAM_RECOVERYFLAG + i, (void *)(buf + i * PARAM_BLKSIZE), PARAM_BLKSIZE);
		else
			ret = param_block_read(PARAM_RECOVERYFLAG + i, (void *)(buf + i * PARAM_BLKSIZE), len);

		if(ret != 0) {
			printf("[Lenovo] Fail to get bootloader_message\n");
			return -1;
		}
		i++;
		len = len - PARAM_BLKSIZE;
	}

	return 0;
}

int write_bootloader_message(void *buf, int len)
{
	int ret = 0, i = 0;

	while (len > 0) {
		if (len >= PARAM_BLKSIZE)
			ret = param_block_write(PARAM_RECOVERYFLAG + i, (void *)(buf + i * PARAM_BLKSIZE), PARAM_BLKSIZE);
		else
			ret = param_block_write(PARAM_RECOVERYFLAG + i, (void *)(buf + i * PARAM_BLKSIZE), len);

		if(ret != 0) {
			printf("[Lenovo] Fail to set bootloader_message\n");
			return -1;
		}
		i++;
		len = len - PARAM_BLKSIZE;
	}

	return 0;
}

int set_bootloader_message(void)
{
	int ret = 0, i = 0;
	int len = sizeof(struct bootloader_message);
	struct bootloader_message *bm = (struct bootloader_message *)malloc(len);

	printf("set recovery flag...\n");
	memset((void *)bm, 0, len);
	strncpy(bm->command, "boot-recovery", sizeof(bm->command));
	strncpy(bm->recovery, "recovery\n"
							RECOVERY_ERASE_DATA
							RECOVERY_ERASE_CACHE
							RECOVERY_SHOW_TEXT, sizeof(bm->recovery));
	printf("%s:%s\n", bm->command, bm->recovery);
	ret = write_bootloader_message((void *)bm, len);
	if(ret != 0) {
		printf("[Lenovo] Fail to set recovery flag\n");
		return -1;
	}

	return 0;
}

int clear_recovery_flag(void)
{
	int ret = 0, i = 0;
	int len = sizeof(struct bootloader_message);
	struct bootloader_message *bm = (struct bootloader_message *)malloc(len);

	printf("clear recovery flag...\n");
	memset((void *)bm, 0, len);
	ret = write_bootloader_message((void *)bm, len);
	if(ret != 0) {
		printf("[Lenovo] Fail to clear recovery flag\n");
		return -1;
	}

	return 0;
}

int recovery_preboot(void)
{
	int ret = 0, i = 0;
	char *str = "boot-recovery";
	int len = sizeof(struct bootloader_message);
	struct bootloader_message *bm = (struct bootloader_message *)malloc(len);

	printf("checking mode for recovery ...\n");
	memset((void *)bm, 0, len);
	ret = get_bootloader_message((void *)bm, len);
	if(ret != 0) {
		printf("[Lenovo] Fail to read recovery flag\n");
		return -1;
	}

#ifdef DEBUG_RECOVERY
	char *p = (char *)bm;
	printf("bootloader_message len : %d\n", len);
	printf("bootloader_message:");
	printf("\n    command:");
	for (i = 0; i < 32; i++)
		printf("%c", *(p+i));
	printf("\n    status:");
	for (i = 32; i < 32 + 32; i++)
		printf("%c", *(p+i));
	printf("\n    recovery:");
	for (i = 32 + 32; i < 32 + 32 + 1024; i++)
		printf("%c", *(p+i));
	printf("\n");
#endif

	if (!memcmp((void *)bm, (void *)str, strlen(str)))
		return 0;

	if(0xABABABAA == get_inform4())
		return 0;

	return -1;
}

int set_recovery_command(char *command, int len)
{
	extern raw_area_t raw_area_control;
	int i = 0, j = 0;

	memset(command, 0, len);

	do {
		if (!strcmp(raw_area_control.image[i].description, "kernel"))
			break;
		i++;
	} while(1);

	do {
		if (!strcmp(raw_area_control.image[j].description, "recovery"))
			break;
		j++;
	} while(1);

	memset(command, 0, sizeof(command));
	sprintf(command, "mmc read 0 40008000 %x %x ;\
		mmc read 0 41000000 %x % x;\
		bootm 40008000 41000000",\
		raw_area_control.image[i].start_blk,\
		raw_area_control.image[i].used_blk,\
		raw_area_control.image[j].start_blk,\
		raw_area_control.image[j].used_blk);

	return 0;
}

extern u32 get_lcd_framebuf(void);
extern int get_lcd_width(void);
int lcd_draw_logo(void)
{
	u32 i, j;
	u32 color;
	u32 *pBuffer = get_lcd_framebuf(); // CFG_LCD_FBUFFER
	int lcd_width = get_lcd_width();		// LCD_WIDTH
	int lcd_height = get_lcd_height();

	unsigned char rbuf[512];
	unsigned short startx, starty;
	unsigned short width, height;

	memset(pBuffer, 0x0, lcd_width*lcd_height*4);
	param_block_read(PARAM_LOGO, rbuf, 12);
	startx = rbuf[4] | (rbuf[5]<<8);
	starty = rbuf[6] | (rbuf[7]<<8);
	width = rbuf[8] | (rbuf[9]<<8);
	height = rbuf[10] | (rbuf[11]<<8);
	rbuf[4] = '\0';
	printf("[Lenovo]TAG: %s, (x, y) = (%u, %u), (w,h) = (%u, %u)\n", \
		rbuf, startx, starty, width, height);

	if(strcmp(rbuf, "LOGO") != 0)
		return -1;

	printf("[Lenovo]Load logo\n");
	pBuffer +=  lcd_width * starty +  startx;
	for (i = 0; i < height; i++)
	{
		memset(rbuf, 0, 512);
		param_block_read(PARAM_LOGO+1+i, rbuf, 512);
		for (j = 0; j < width; j++)
		{
			color = rbuf[j];
			color = (color << 16) | (color << 8) | color;
			*pBuffer++ = color;
		}
		pBuffer += lcd_width - width;
	}

	return 0;
}
