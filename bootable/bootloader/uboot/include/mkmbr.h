#define LINUX_NATIVE            0x83
#define EXTENDED                0x05
#define FAT32					0x0C
#define BLOCK_SIZE				512

typedef unsigned int sector_t;
struct partition {
	unsigned char boot_ind;         /* 0x80 - active */
	unsigned char head;             /* starting head */
	unsigned char sector;           /* starting sector */
	unsigned char cyl;              /* starting cylinder */
	unsigned char sys_ind;          /* what partition type */
	unsigned char end_head;         /* end head */
	unsigned char end_sector;       /* end sector */
	unsigned char end_cyl;          /* end cylinder */
	unsigned char start4[4];        /* starting sector counting from 0 */
	unsigned char size4[4];         /* nr of sectors in partition */
};

/*
 * per partition table entry data
 *
 * The four primary partitions have the same sectorbuffer (MBRbuffer)
 * and have NULL ext_pointer.
 * Each logical partition table entry has two pointers, one for the
 * partition and one link to the next one.
 */
struct pte {
	struct partition *part_table;   /* points into sectorbuffer */
	struct partition *ext_pointer;  /* points into sectorbuffer */
	sector_t offset_from_dev_start; /* disk sector number */
	unsigned char sectorbuffer[512];         /* disk sector contents */
	char changed;                   /* boolean */
};

extern struct pte ptes[MAX_PTN];
extern fastboot_ptentry *ptable_default;
extern unsigned int ptable_default_size;
//extern fastboot_ptentry *ptable;
extern struct fastboot_ptentry ptable_mbr[MAX_PTN];
extern unsigned int partition_num;

extern int mkmbr(int argc, char *argv[]);
