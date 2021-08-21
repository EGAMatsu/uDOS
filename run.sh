export PATH="$PATH:$HOME/src/uDOS/s390-linux/bin"
make -j || exit

#qemu-system-s390x \
#    -drive if=none,file=disk.img,id=hd0,format=raw \
#    -device virtio-scsi,id=scsi0 -device scsi-hd,drive=hd0 \
#    -kernel kernel/kernel \
#    -smp 4 \
#    -trace css\* -trace s390\* -trace guest_cpu\* \
#    -trace qxl\* -trace vfio\* -trace qemu_vfio\* \
#    -d guest_errors,nochain

rm udos00.cckd

s390-linux-objcopy -O binary ipl_loader/ipl_loader ipl_loader/ipl_loader.bin || exit
./bin2txt ipl_loader/ipl_loader.bin ipl_loader.txt || exit

s390-linux-objcopy -O binary kernel/kernel kernel/kernel.bin || exit
./bin2txt kernel/kernel.bin kernel.txt || exit

dasdload -bz2 ctl.txt udos00.cckd || exit
hercules -f udos.cnf >hercules.log