make
./mkpad
echo "cat E4412_S.SSCR.bl1, E4412_bl2.bin, bl2.2k.pad, u-boot.bin E4412_tzsw.bin.KH to u-boot-emmc-tz.bin"
cat E4412_S.SSCR.bl1 E4412_bl2.bin.KH bl2.2k.pad ../u-boot.bin E4412_tzsw.bin.KH >u-boot-emmc-tz.bin
echo "u-boot-emmc-tz.bin successfully made to support TrustZone"
