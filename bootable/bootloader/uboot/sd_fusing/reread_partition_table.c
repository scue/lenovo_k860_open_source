#include <stdio.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>


int main(int argc, char *argv[])
{
	int ret = 0;
	int fd = 0;

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("open %s error\n", argv[1]);
		return fd;
	}

	ret = ioctl(fd, BLKRRPART);
	if (ret != 0) {
		printf("ioctl error, ret = %d\n", ret);
		return ret;
	}


	ret = close(fd);
	if (ret != 0) {
		printf("close error, ret = %d\n", ret);
		return ret;
	}

	return 0;
}
