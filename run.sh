export PATH="$PATH:$HOME/opt/cross/s390-linux/bin"

make -j || exit

rm udos00.cckd
s390-linux-objcopy -O binary ipl_loader/ipl_loader ipl_loader/ipl_loader.bin || exit
./tools/bin2rec ipl_loader/ipl_loader.bin ipl_loader.txt || exit

s390-linux-objcopy -O binary kernel/kernel kernel/kernel.bin || exit
./tools/bin2rec kernel/kernel.bin kernel.txt || exit

dasdload -bz2 ctl.txt udos00.cckd || exit
hercules -f udos.cnf >hercules.log || exit