export PATH="$PATH:$HOME/opt/cross/s390-linux/bin"

make -j || exit
rm udos00.cckd

s390-linux-objcopy -O binary kernel/kernel kernel/kernel.bin || exit
s390-linux-objcopy -O binary ipl/ipl ipl/ipl.bin || exit
./tools/bin2rec ipl/ipl.bin ipl.txt

dasdload -bz2 ctl.txt udos00.cckd || exit
hercules -f udos.cnf >hercules.log || exit