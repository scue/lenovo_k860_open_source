#include <common.h>
#include <command.h>
#include <mmc.h>
#include <fastboot.h>
#include <mkmbr.h>

/* #define DEBUG_MBR */
#define MAX_PTN 16
struct fastboot_ptentry ptable_mbr[MAX_PTN];
struct pte ptes[MAX_PTN];

int dos_compatible_flag;
unsigned int g_cylinders;
unsigned int g_heads;
unsigned int g_sectors;
unsigned int extended_offset;
unsigned int sector_offset;
unsigned int partition_num;

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

static inline int le32_to_buf(int le32, unsigned char *buf)
{
	buf[0] = le32 & 0xff;
	buf[1] = (le32  >> 8) & 0xff;
	buf[2] = (le32  >> 16) & 0xff;
	buf[3] = (le32  >> 24) & 0xff;

	return 0;
}

static void set_start_sect(struct partition *p, unsigned start_sect)
{
	le32_to_buf(start_sect, p->start4);
}

static void set_nr_sects(struct partition *p, unsigned nr_sects)
{
	le32_to_buf(nr_sects, p->size4);
}

#define set_hsc(h, s, c, sector) do \
{ \
	s = sector % g_sectors + 1;  \
	sector /= g_sectors;         \
	h = sector % g_heads;        \
	sector /= g_heads;           \
	c = sector & 0xff;           \
	s |= (sector >> 2) & 0xc0;   \
} while (0)

#define pt_offset(b, n) \
	((struct partition *)((b) + 0x1be + (n) * sizeof(struct partition)))

static void set_hsc_start_end(struct partition *p, sector_t start, sector_t stop)
{
	if (dos_compatible_flag && (start / (g_sectors * g_heads) > 1023))
		start = g_heads * g_sectors * 1024 - 1;
	set_hsc(p->head, p->sector, p->cyl, start);

	if (dos_compatible_flag && (stop / (g_sectors * g_heads) > 1023))
		stop = g_heads * g_sectors * 1024 - 1;
	set_hsc(p->end_head, p->end_sector, p->end_cyl, stop);
}

int write_part_table_flag(unsigned char *b)
{
	b[510] = 0x55;
	b[511] = 0xaa;

	return 0;
}

static void set_partition(int i, int doext, sector_t start, sector_t stop, int sysid)
{
	struct partition *p;
	sector_t offset;

	if (doext) {
		p = ptes[i].ext_pointer;
		offset = extended_offset;
	} else {
		p = ptes[i].part_table;
		offset = ptes[i].offset_from_dev_start;
	}
	p->boot_ind = 0;
	p->sys_ind = sysid;
	set_start_sect(p, start - offset);
	set_nr_sects(p, stop - start + 1);
	set_hsc_start_end(p, start, stop);
	ptes[i].changed = 1;
}

int init_ptable_mbr(int block_count)
{
	int i;

	memcpy((void*)ptable_mbr, (void*)&ptable_default,
			3 * sizeof(struct fastboot_ptentry));
	memcpy((void*)((char *)ptable_mbr + sizeof(struct fastboot_ptentry) * 4),
			(void*)((char *)&ptable_default + sizeof(struct fastboot_ptentry) * 3),
			ptable_default_size - 3 * sizeof(struct fastboot_ptentry));

	ptable_mbr[3].start = ptable_mbr[4].start;
	partition_num = ptable_default_size/sizeof(struct fastboot_ptentry) + 1;
	ptable_mbr[3].length = 0;
	for (i = 4; i < partition_num; i++)
		ptable_mbr[3].length += ptable_mbr[i].length;

	ptable_mbr[0].length = (unsigned long long) block_count * BLOCK_SIZE;
	for(i = 1; i < 4; i++)
		ptable_mbr[0].length -= ptable_mbr[i].length;

	/* delete the raw_area size */
	ptable_mbr[0].length -= ptable_mbr[1].start;

#ifdef DEBUG_MBR
	for (i = 0; i < partition_num; i++) {
		printf("%d:start=0x%llx", i, ptable_mbr[i].start);
		printf("length=0x%llx\n", ptable_mbr[i].length);
	}
#endif

	return 0;
}

int init_ptes(void)
{
	int i;
	for (i = 0; i < 4; i++) {
		ptes[i].part_table = pt_offset(ptes[3].sectorbuffer, i);
		ptes[i].ext_pointer = NULL;
		ptes[i].offset_from_dev_start = 0;
	}
	for (i = 4; i < partition_num; i++) {
		ptes[i].part_table = pt_offset(ptes[i].sectorbuffer, 0);
		ptes[i].ext_pointer = ptes[i].part_table + 1;
		ptes[i].offset_from_dev_start = ptable_mbr[i].start/BLOCK_SIZE;
	}

	return 0;
}

int mkmbr(int argc, char *argv[])
{
	int i, j;
	unsigned int start = 0, stop = 0;
	int sys_ind = 0;

	int total_block_count;
	SDInfo sdinfo;

	total_block_count = get_mmc_block_count(argv[2]);
	if (total_block_count < 0)
		return -1;

	get_SDInfo(total_block_count, &sdinfo);

	dos_compatible_flag = 1;
	/*g_cylinders = 1021;
	g_heads = 255;
	g_sectors = 60;*/
	g_cylinders = sdinfo.C_end;
	g_heads = sdinfo.H_end;
	g_sectors = sdinfo.S_end;
	printf("g_cylinders:%d,g_heads:%d,g_sectors:%d\n", g_cylinders, g_heads, g_sectors);
	sector_offset = g_sectors;

	init_ptable_mbr(total_block_count);
	init_ptes();

	extended_offset = ptable_mbr[3].start/BLOCK_SIZE;
	for (i = 0; i < partition_num ; i++) {
		start = ptable_mbr[i].start/BLOCK_SIZE;
		stop = (unsigned long long)(ptable_mbr[i].start + ptable_mbr[i].length - BLOCK_SIZE)/BLOCK_SIZE;
		if (strcmp(ptable_mbr[i].name, "fat") == 0)
			sys_ind = FAT32;
		else
			sys_ind = LINUX_NATIVE;
		if (i == 3)
			set_partition(i, 0, start, stop, EXTENDED);
		else if (i > 3)
			set_partition(i, 0, start + sector_offset, stop, sys_ind);
		else
			set_partition(i, 0, start, stop, sys_ind);

		if (i > 4) {
			set_partition(i - 1, 1, start, stop, EXTENDED);
		}

	}

	for (i = 3; i < partition_num; i++)
		write_part_table_flag(ptes[i].sectorbuffer);
#ifdef DEBUG_MBR
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

	return 0;
}
