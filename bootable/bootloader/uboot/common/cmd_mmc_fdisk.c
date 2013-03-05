/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * fdisk command for U-boot
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <mmc.h>
#include <fastboot.h>
#include <mkmbr.h>

#define		BLOCK_SIZE			512
#define		_8_4GB				(1023*254*63)

#define		CHS_MODE			0
#define		LBA_MODE			!(CHS_MODE)

typedef struct
{
	int		C_start;
	int		H_start;
	int		S_start;

	int		C_end;
	int		H_end;
	int		S_end;

	int		available_block;
	int		unit;
	int		total_block_count;
	int		addr_mode;	// LBA_MODE or CHS_MODE
} SDInfo;

typedef struct
{
	unsigned char bootable;
	unsigned char partitionId;

	int		C_start;
	int		H_start;
	int		S_start;

	int		C_end;
	int		H_end;
	int		S_end;

	int		block_start;
	int		block_count;
	int		block_end;
} PartitionInfo;

/////////////////////////////////////////////////////////////////
int calc_unit(unsigned long long length, SDInfo sdInfo)
{
	if (sdInfo.addr_mode == CHS_MODE)
		return ( (length / BLOCK_SIZE / sdInfo.unit + 1 ) * sdInfo.unit);
	else
		return ( (length / BLOCK_SIZE) );
}

/////////////////////////////////////////////////////////////////
void encode_chs(int C, int H, int S, unsigned char *result)
{
	*result++ = (unsigned char) H;
	*result++ = (unsigned char) ( S + ((C & 0x00000300) >> 2) );
	*result   = (unsigned char) (C & 0x000000FF); 
}

/////////////////////////////////////////////////////////////////
void encode_partitionInfo(PartitionInfo partInfo, unsigned char *result)
{
	*result++ = partInfo.bootable;

	encode_chs(partInfo.C_start, partInfo.H_start, partInfo.S_start, result);
	result +=3;
	*result++ = partInfo.partitionId;

	encode_chs(partInfo.C_end, partInfo.H_end, partInfo.S_end, result);
	result += 3;

	memcpy(result, (unsigned char *)&(partInfo.block_start), 4);
	result += 4;	
	
	memcpy(result, (unsigned char *)&(partInfo.block_count), 4);
}

/////////////////////////////////////////////////////////////////
void decode_partitionInfo(unsigned char *in, PartitionInfo *partInfo)
{
	partInfo->bootable	= *in;
	partInfo->partitionId	= *(in + 4); 

	memcpy((unsigned char *)&(partInfo->block_start), (in + 8), 4);
	memcpy((unsigned char *)&(partInfo->block_count), (in +12), 4);
}

/////////////////////////////////////////////////////////////////
void get_SDInfo(int block_count, SDInfo *sdInfo)
{
       int C, H, S;

        int C_max = 1023, H_max = 255, S_max = 63;
        int H_start = 1, S_start = 1;
        int diff_min = 0, diff = 0;

        if(block_count >= _8_4GB)
                sdInfo->addr_mode = LBA_MODE;
        else
                sdInfo->addr_mode = CHS_MODE;

//-----------------------------------------------------
        if (sdInfo->addr_mode == CHS_MODE)
        {
                diff_min = C_max;

                for (H = H_start; H <= H_max; H++)
                        for (S  = S_start; S <= S_max; S++)
                        {
                                C = block_count / (H * S);

                                if ( (C <= C_max) )
                                {
                                        diff = C_max - C;
                                        if (diff <= diff_min)
                                        {
                                                diff_min = diff;
                                                sdInfo->C_end = C;
                                                sdInfo->H_end = H;
                                                sdInfo->S_end = S;
                                        }
                                }
                        }
        }
//-----------------------------------------------------
        else
        {
                sdInfo->C_end = 1023;
                sdInfo->H_end = 254;
                sdInfo->S_end = 63;
        }

//-----------------------------------------------------
        sdInfo->C_start                 = 0;
        sdInfo->H_start                 = 1;
        sdInfo->S_start                 = 1;

        sdInfo->total_block_count       = block_count;
        sdInfo->available_block         = sdInfo->C_end * sdInfo->H_end * sdInfo->S_end;
        sdInfo->unit                    = sdInfo->H_end * sdInfo->S_end;
}

/////////////////////////////////////////////////////////////////
int get_mmc_block_count(char *device_name)
{
	int rv;
	struct mmc *mmc;
	int block_count = 0;
	int dev_num;

	dev_num = simple_strtoul(device_name, NULL, 0);
	
	mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		printf("mmc/sd device is NOT founded.\n");
		return -1;
	}	
	
	block_count = mmc->capacity * (mmc->read_bl_len / BLOCK_SIZE);
		
//	printf("block_count = %d\n", block_count);
	return block_count;
}

/////////////////////////////////////////////////////////////////
int get_mmc_mbr(char *device_name, unsigned char *mbr)
{
	int rv;
	struct mmc *mmc;
	int dev_num;

	dev_num = simple_strtoul(device_name, NULL, 0);
	
	mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		printf("mmc/sd device is NOT founded.\n");
		return -1;
	}	
	
	rv = mmc->block_dev.block_read(dev_num, 0, 1, mbr);

	if(rv == 1)
		return 0;
	else
		return -1; 
}

/////////////////////////////////////////////////////////////////
int put_mmc_mbr(int argc, char *argv[])
{
	int rv;
	struct mmc *mmc;
	int dev_num;
	unsigned int mbr_addr = 0;
	int i;
	unsigned char buf[512];
	struct pte *pe = &ptes[i];
	char *ep;

	dev_num = (int)simple_strtoul(argv[2], &ep, 16);

	mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		printf("mmc/sd device is NOT founded.\n");
		return -1;
	}	

	for (i = 0; i < 3; i++)
		if (ptes[i].changed)
			ptes[3].changed = 1;

	for (i = 3; i < partition_num; i++) {
		if (pe->changed) {
			if (i <= 3)
				mbr_addr = 0;
			else if( i > 3)
				mbr_addr = ptable_mbr[i].start/BLOCK_SIZE;
			rv = mmc->block_dev.block_write(dev_num, mbr_addr, 1, ptes[i].sectorbuffer);
			if(rv != 1) {
				printf("put mmc mbr error\n");
				return -1;
			}
			/* Verify mbr */
			rv = mmc->block_dev.block_read(dev_num, mbr_addr, 1, buf);
			if(rv != 1) {
				printf("get mmc mbr error\n");
				return -1;
			}
			if (strncmp(ptes[i].sectorbuffer, buf, 512) != 0) {
				printf("Verify MBR error\n");
				return -1;
#ifdef DEBUG_FDIDSK
			} else {
				/* show mbr info */
				int j;
				for (i = 3; i < partition_num; i++) {
					printf("*******mbr-%d*********\n", i - 3);
					for (j = 0x1be; j < 512; j++) {
						printf("0x%02X, ", ptes[i].sectorbuffer[j]);
						if ((j + 1) % 16 == 0)
							printf("\n");
					}
					printf("\n");
				}
#endif
			}
		}
	}

	init_part(&mmc->block_dev);
	return 0;
}

/////////////////////////////////////////////////////////////////
int get_mmc_part_info(char *device_name, int part_num, int *block_start, int *block_count, unsigned char *part_Id)
{
	int		rv;
	PartitionInfo	partInfo;
	unsigned char	mbr[512];
	
	int i = 0, dev = 0;
	disk_partition_t part;
	block_dev_desc_t *dev_desc = NULL;
	dev_desc = get_dev("mmc", dev);

	rv = get_partition_info (dev_desc, part_num, &part);
	if (rv != 0)
		return rv;
	*block_start = part.start;
	*block_count = part.size;
	*part_Id = part.sys_ind;

	return 0;
}

/////////////////////////////////////////////////////////////////
int print_mmc_part_info(int argc, char *argv[])
{
	int dev = 0;
	int i = 0;
	char *ep;

	disk_partition_t part;
	block_dev_desc_t *dev_desc = NULL;
	dev = (int)simple_strtoul(argv[2], &ep, 16);
	dev_desc = get_dev("mmc", dev);

	if (dev_desc == NULL) {
		puts("\n** Invalid boot device **\n");
		return 1;
	}

	print_part_dos(dev_desc);

	return 0;
}

/////////////////////////////////////////////////////////////////
int create_mmc_fdisk(int argc, char *argv[])
{

	mkmbr(argc, argv);
	put_mmc_mbr(argc, argv);
	print_mmc_part_info(argc, argv);
	return 0;
}

/////////////////////////////////////////////////////////////////
int do_fdisk(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if ( argc == 3 || argc ==6 )
	{
		if ( strcmp(argv[1], "-c") == 0 )
			return create_mmc_fdisk(argc, argv);

		else if ( strcmp(argv[1], "-p") == 0 )
			return print_mmc_part_info(argc, argv);
	}
	else
	{
		printf("Usage:\nfdisk <-p> <device_num>\n");
		printf("fdisk <-c> <device_num> [<sys. part size(MB)> <user data part size> <cache part size>]\n");
	}
	return 0;
}

U_BOOT_CMD(
	fdisk, 6, 0, do_fdisk,
	"fdisk\t- fdisk for sd/mmc.\n",
	"-c <device_num>\t- create partition.\n"
	"fdisk -p <device_num> [<sys. part size(MB)> <user data part size> <cache part size>]\t- print partition information\n"
);

int check_partition_table_and_fdisk(int dev_num)
{
	int rv = 0;
	char *run_cmd[80];

	printf("fdisk partition\n");
	sprintf(run_cmd, "fdisk -c %d", dev_num);
	rv = run_command(run_cmd, 0);
	if (rv) {
		printf("fdisk error\n");
		return 1;
	}

	return 0;
}
