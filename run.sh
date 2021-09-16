#!/bin/bash

# Default stuff
target="s390-linux"
memory_size="32"
n_cpus="1"
disk_file="udos00.cckd"

function cmd_help {
    echo "Usage:"
    echo "    -p <directory>      Cross compiler path to use"
    echo "    -t <directory>      Target to use (default is $target)"
    echo "                        s390-linux"
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

make -j || exit
if [ -f $disk_file ]; then
    rm $disk_file
fi

$target-objcopy -O binary kernel/kernel kernel/kernel.bin || exit
if [ -z $ipl_file ]; then
    $target-objcopy -O binary ipl/ipl ipl/ipl.bin || exit
    ./tools/bin2rec ipl/ipl.bin ipl.txt || exit
else
    if [ ! -f "$ipl_file" ]; then
        echo "$ipl_file does not exist"
        exit
    fi
    ./tools/bin2rec $ipl_file ipl.txt || exit
fi

dasdload -bz2 ctl.txt $disk_file || exit

cat udos.cnf | awk '!/^#/ {
    if($1 == "NUMCPU") print "NUMCPU               '"$n_cpus"'";
    else if($1 == "MAINSIZE") print "MAINSIZE             '"$memory_size"'";
    else if($2 == "3390") print "01b9      3390       '"$disk_file"'";
    else if($1 != "") print $0; }' >autogen.cnf
cat autogen.cnf

hercules -f autogen.cnf >hercules.log || exit

# Hercules messes up colours so we have to clean it up
printf '\x1b[0;0m'

rm autogen.cnf