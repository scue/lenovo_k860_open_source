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


#define BL2_SIZE (14 * 1024)

static void movi_write_bl2(void)
{
        int     i;
        ulong   checksum=0;
        unsigned char      *addr;
        unsigned char      *paddr;

	FILE *fp_uboot = NULL;
	FILE *fp_bl2ah = NULL;

	fp_uboot = fopen("../u-boot.bin", "rb");
	fp_bl2ah = fopen("bl2.bin", "wb");

        addr = (unsigned char *) malloc(BL2_SIZE);
        paddr = addr;
	memset(addr, 0, BL2_SIZE );

	fread(addr, 1, BL2_SIZE, fp_uboot);

        for(i = 0, checksum = 0;i < (14 * 1024) - 4;i++)
        {
                checksum += *(unsigned char*)addr++;
        }

        *(ulong*)addr = checksum;

	fwrite(paddr, 1, BL2_SIZE, fp_bl2ah);

	fclose(fp_uboot);
	fclose(fp_bl2ah);

        free(paddr);
}


int main(int argc, char *argv[])
{

	movi_write_bl2();
	return 0;
}

