export PATH="$PATH:$HOME/opt/cross/s390-linux/bin"

if ! test -f ipl_loader/stivale.h; then
    wget https://github.com/stivale/stivale/raw/master/stivale.h
    mv stivale.h ipl_loader/stivale.h
fi

if ! test -f ipl_loader/stivale2.h; then
    wget https://github.com/stivale/stivale/raw/master/stivale2.h
    mv stivale2.h ipl_loader/stivale2.h
fi

make -j || exit
rm udos00.cckd

s390-linux-objcopy -O binary ipl_loader/ipl_loader ipl_loader/ipl_loader.bin || exit
./tools/bin2rec ipl_loader/ipl_loader.bin ipl_loader.txt || exit
s390-linux-objcopy -O binary ipl_loader/stivale ipl_loader/stivale.bin || exit
s390-linux-objcopy -O binary kernel/kernel kernel/kernel.bin || exit

dasdload -bz2 ctl.txt udos00.cckd || exit
hercules -f udos.cnf >hercules.log || exit