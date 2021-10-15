#!/bin/bash

# Default stuff
target="s390-linux"
memory_size="2068"
n_cpus="1"
disk_file="udos00.cckd"

function cmd_help {
    echo "Usage:"
    echo "    -p <directory>      Cross compiler path to use"
    echo "    -t <directory>      Target to use (default is $target)"
    echo "                        s390-linux, xtensa-linux"
    echo "    -i <file>           Use an alternate IPL"
    echo "    -m <size>           Specify a memory size (in MiB)"
    echo "    -j <n_cpus>         Number of cores to run at once"
    echo "    -d <file>           Specify a disk file"
    exit
}

while getopts p:t:i:h:m:j: flag; do
    case ${flag} in
        m) memory_size=${OPTARG};;
        j) n_cpus=${OPTARG};;
        p) cross_path=${OPTARG};;
        t) target=${OPTARG};;
        i) ipl_file=${OPTARG};;
        d) disk_file=${OPTARG};;
        h) cmd_help;;
    esac
done

# In case of no cross path specified we just use something that "may" work
# We do not initialize since they may specify a target and our path may just match
if [ -z $cross_path ]; then
    cross_path="$HOME/opt/cross/$target/bin"
fi

export PATH="$PATH:$cross_path"

# Make a symbolic link
cd build
make clean || exit
make -j || exit
if [ -f $disk_file ]; then
    rm $disk_file
fi

cp ../ctl.txt .

$target-objcopy -O binary kernel/kernel kernel/kernel.bin || exit
split -b 18452 kernel/kernel.bin --verbose

case "${target}" in
    s3*0* | zarch*)
        dasdload -bz2 ctl.txt $disk_file || exit
        hercules -f hercules.cnf >hercules.log || exit

        # Hercules messes up colours so we have to clean it up
        printf '\x1b[0;0m'
    ;;
    xtensa*)
        qemu-system-xtensa -kernel kernel/kernel
    ;;
    *)
        echo "Unknown emulator for ${target}!"
    ;;
esac
cd ..