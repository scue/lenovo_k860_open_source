#include <configs/stuttgart.h>
#include "bootimg.h"

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))

int boot_linux_from_mmc(int partition_no)
{
	unsigned char buf[4096];
	struct boot_img_hdr *hdr = (void*)buf;
	struct boot_img_hdr *uhdr;
	unsigned offset = 0;
	unsigned long long ptn = 0;
	unsigned n = 0;
	const char *cmdline;

	unsigned char *image_addr = 0;
	unsigned kernel_actual;
	unsigned ramdisk_actual;
	unsigned imagesize_actual;

	unsigned page_size = 0;
	unsigned page_mask = 0;

	unsigned char command[100];

	unsigned long long start, count;
	unsigned char pid;

	get_mmc_part_info("0", partition_no, &start, &count, &pid);

	offset = start;

	memset(command, 0, sizeof(command));
	sprintf(command, "mmc read 0 %p %x %x", buf, offset, 4);
	run_command(command, 0);

	if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
		printf("ERROR: Invalid boot image header\n");
		return -1;
	}
	if (hdr->page_size && (hdr->page_size != page_size)) {
		page_size = hdr->page_size;
		page_mask = page_size - 1;
	}

	offset += page_size/CFG_FASTBOOT_SDMMC_BLOCKSIZE;
	n = ROUND_TO_PAGE(hdr->kernel_size, page_mask);
	memset(command, 0, sizeof(command));
	sprintf(command, "mmc read 0 40008000 %x %x",\
			offset, (n/CFG_FASTBOOT_SDMMC_BLOCKSIZE));
	run_command(command, 0);

	offset += n/CFG_FASTBOOT_SDMMC_BLOCKSIZE;
	n = ROUND_TO_PAGE(hdr->ramdisk_size, page_mask);
	if(n != 0)
	{
		memset(command, 0, sizeof(command));
		sprintf(command, "mmc read 0 41000000 %x %x",\
				offset, (n/CFG_FASTBOOT_SDMMC_BLOCKSIZE));
		run_command(command, 0);
	}
	offset += n;

	printf("\nkernel  @ %x (%d bytes)\n", hdr->kernel_addr,
			hdr->kernel_size);
	printf("ramdisk @ %x (%d bytes)\n", hdr->ramdisk_addr,
			hdr->ramdisk_size);
	printf("\nBooting Linux\n");

	/* run_command("bootm 40008000 41000000", 0); */

	return 0;
}


