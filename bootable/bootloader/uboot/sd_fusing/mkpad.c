/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define BL2_SIZE (14 * 1024)
#define UBOOT_SIZE (328 * 1024)

int get_file_size(char *filename)
{
	struct stat sb;

	if (stat(filename, &sb)  ==  -1) {
		return -1;
	}

	printf("File size:                %lld bytes\n",
			(long long) sb.st_size);

	return sb.st_size;
}

int movi_uboot_pad(void)
{
	int i;
	unsigned char val = 0;
	FILE *fp_uboot = NULL;
	long long delta_size = 0;
	long long uboot_size = 0;

	uboot_size = get_file_size("../u-boot.bin");

	delta_size = UBOOT_SIZE - uboot_size;
	if (delta_size < 0){
		printf("u-boot.bin is to large\n");
		return -1;
	}

	fp_uboot = fopen("../u-boot.bin", "a+b");
	if (fp_uboot == NULL){
		printf("file open err\n");
		return;
	}

	for (i = 0; i < delta_size; i++)
		fwrite(&val, 1, 1, fp_uboot);

	fclose(fp_uboot);

	return 0;
}

static void movi_bl2_pad(void)
{
        int     i;
        unsigned char      val = 0;

	FILE *fp_bl2ah = NULL;

	fp_bl2ah = fopen("bl2.bin.signed", "a+b");
	if (fp_bl2ah == NULL){
		printf("file open err\n");
		return;
	}
	for (i = 0; i < (16 * 1024 -256 - 14 *1024); i++)
		fwrite(&val, 1, 1, fp_bl2ah);

	fclose(fp_bl2ah);

}
int main(int argc, char *argv[])
{

	movi_bl2_pad();
	movi_uboot_pad();
	return 0;
}

