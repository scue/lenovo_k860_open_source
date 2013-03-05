#!/bin/bash
#
# Copyright (C) 2010 Samsung Electronics Co., Ltd.
#              http://www.samsung.com/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
####################################

if [ -z $1 ]
then
    echo "usage: ./sd_fusing.sh <SD Reader's device file>"
    exit 0
fi

if [ -b $1 ]
then
    echo "$1 reader is identified."
else
    echo "$1 is NOT identified."
    exit 0
fi


####################################
# umount $11
echo "umount $11 if it is mounted"
MOUNT_DEV=`cat /proc/mounts | cut -f1 --delimiter=" "`
for i in $MOUNT_DEV
do
	if [ $i = $11 ]; then
		echo $11 has been mounted, umount it
		umount $11
	fi
done
####################################

# make partition
echo "make sd card partition"
echo "./sd_fdisk $1"
./sd_fdisk $1
dd iflag=dsync oflag=dsync if=sd_mbr.dat of=$1
rm sd_mbr.dat
sync
./reread_partition_table $1
echo "mkfs.vfat -F 32 $11"
mkfs.vfat -F 32 $11
####################################

signed_bl1_position=1
bl2_position=17
uboot_position=49
tzsw_position=705

echo "BL1 fusing"
dd iflag=dsync oflag=dsync if=E4412_S.SSCR.bl1 of=$1 seek=$signed_bl1_position

####################################
echo "TrustZone BL2 fusing"
dd iflag=dsync oflag=dsync if=E4412_bl2.bin.KH of=$1 seek=$bl2_position 

#<u-boot fusing>
echo "u-boot fusing"
dd iflag=dsync oflag=dsync if=../u-boot.bin of=$1 seek=$uboot_position

#<TrustZone S/W fusing>
echo "TrustZone S/W fusing"
dd iflag=dsync oflag=dsync if=./E4412_tzsw.bin.KH of=$1 seek=$tzsw_position

####################################
#<Message Display>
echo "U-boot image is fused successfully."
echo "Eject SD card and insert it again."
