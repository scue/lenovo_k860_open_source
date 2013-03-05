make
echo "make bl2"
./mk_bl2_security
chmod 777 bl2.bin.signed
./codesigner_v21 -v2.1 bl2.bin bl2.bin.signed stage2Key_V21.prv -STAGE2
./codesigner_v21 -v2.1 bl2.bin.signed stage2Key_V21.spk -STAGE2_VERIFY
echo "make bl2 signed"
./mkpad
#rm bl2.bin
#rm bl2.bin.signed
echo "bl2.bin.signed has been generated successfully"
cat E4412_S.SSCR.bl1 bl2.bin.signed ../u-boot.bin >u-boot-emmc.bin
