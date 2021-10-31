#!/bin/bash

# Default stuff
build_target="s390-linux"

memory_size="2068"
n_cpus="1"
disk_file="udos00.cckd"

# In case of no cross path specified we just use something that "may" work
# We do not initialize since they may specify a target and our path may just match
if [ -z $cross_path ]; then
    cross_path="$HOME/opt/cross/$build_target/bin"
fi

export PATH="$PATH:$cross_path"

# Make a symbolic link
cd Build
#make clean || exit
make -j || exit
if [ -f $disk_file ]; then
    rm $disk_file
fi

cp ../ctl.txt .

$build_target-objcopy -O binary doskrnl/doskrnl doskrnl/doskrnl.bin || exit

case "${build_target}" in
    s3*0* | zarch*)
        dasdload -bz2 ctl.txt $disk_file || exit
        hercules -f hercules.cnf >hercules.log || exit

        # Hercules messes up colours so we have to clean it up
        printf '\x1b[0;0m'
    ;;
    xtensa*)
        qemu-system-xtensa -kernel doskrnl/doskrnl
    ;;
    *)
        echo "Unknown emulator for ${build_target}!"
    ;;
esac
cd ..
