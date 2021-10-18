memory_size="2068"
n_cpus="1"
disk_file="udos00.cckd"

mkdir -p build
cd build

# ******************************************************************************
# PERSONAL COMPUTING EQUIPMENT BUILDS
# ******************************************************************************

# i686
PATH="$PATH:$HOME/opt/cross/i686-linux/bin"
target="i686-linux"
mkdir -p x32
cd x32
../../configure --host=i686-linux --target=i686-linux || exit
make -j$(nproc) || exit
cd ..

# x86_64
PATH="$PATH:$HOME/opt/cross/x86_64-linux/bin"
target="x86_64-linux"
mkdir -p x64
cd x64
../../configure --host=x86_64-linux --target=x86_64-linux || exit
make -j$(nproc) || exit
cd ..

# PA-RISC workstations
#PATH="$PATH:$HOME/opt/cross/hppa-linux/bin"
#target="hppa-linux"
#mkdir -p parisc
#cd parisc
#../../configure --host=hppa-linux --target=hppa-linux || exit
#make -j$(nproc) || exit
#cd ..

# PA-RISC 64-bit workstations
#PATH="$PATH:$HOME/opt/cross/hppa64-linux/bin"
#target="hppa64-linux"
#mkdir -p parisc64
#cd parisc64
#../../configure --host=hppa64-linux --target=hppa64-linux || exit
#make -j$(nproc) || exit
#cd ..

# ******************************************************************************
# IBM ESA/3X0 MAINFRAME BUILDS
# ******************************************************************************
PATH="$PATH:$HOME/opt/cross/s390-linux/bin"

# ESA 360
target="s390-linux"
mkdir -p esa360
cd esa360
../../configure --host=s390-linux --target=s390-linux || exit
make -j$(nproc) || exit
cd ..

# ESA 370
target="s370-linux"
mkdir -p esa370
cd esa370
../../configure --host=s390-linux --target=s390-linux || exit
make -j$(nproc) || exit
cd ..

# ESA 390
target="s390-linux"
mkdir -p esa390
cd esa390
../../configure --host=s390-linux --target=s390-linux || exit
make -j$(nproc) || exit
cd ..

# z/Architecture
target="zarch-linux"
mkdir -p zarch
cd zarch
../../configure --host=s390-linux --target=s390-linux || exit
make -j$(nproc) || exit
cd ..

# ******************************************************************************
# EMBEDDED PROCESSOR BUILDS
# ******************************************************************************

# Xtensa family
PATH="$PATH:$HOME/opt/cross/xtensa-linux/bin"

# xtensa-esp32
target="xtensa-esp32-linux"
mkdir -p xtensa-esp32
cd xtensa-esp32
../../configure --host=xtensa-linux --target=xtensa-linux || exit
make -j$(nproc) || exit
cd ..

target="xtensa-lx106-linux"
mkdir -p xtensa-lx106
cd xtensa-lx106
../../configure --host=xtensa-linux --target=xtensa-linux || exit
make -j$(nproc) || exit
cd ..

cd ..