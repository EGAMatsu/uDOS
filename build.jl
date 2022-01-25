#!/bin/julia

#
# Some julia build system for UDOS
# somewhat better than using raw batch files for tracking stuff
#
# Why julia? well because-
#

const src_dir = pwd()
if src_dir == "/" || src_dir == "c:/"
    @error "Don't use your root directory!"
end
const ds_dir = "$src_dir/distro"
const mvs_prefix = "c:/pdos/mvs380"

# This can fail if the directory does not exist
@info "Removing old distrobution directory"
rm("$ds_dir", recursive = true, force = true)
mkdir("$ds_dir")

@info "Collecting .c, .asm and .h source code files"
ASM_Files = String[]
C_Files = String[]
CHeader_Files = String[]
source_files = readdir("$src_dir/kernel", join = true, sort = true)
for filepath in source_files
    ext = splitext(filepath)[2]
    if ext == ".c"
        push!(C_Files, filepath)
    elseif ext == ".h"
        push!(CHeader_Files, filepath)
    elseif ext == ".asm"
        push!(ASM_Files, filepath)
    end
end

@info "Archive C and ASM source code"
run(`zip -9 -X -ll $ds_dir/source.zip $src_dir/kernel/\*.asm $src_dir/kernel/\*.c`)
@info "Archive header files"
run(`zip -9 -X -ll $ds_dir/include.zip $src_dir/kernel/\*.h`)
@info "Archive JCL deck cards"
run(`zip -9 -X -ll $ds_dir/jcl.zip $src_dir/jcl/\*.jcl`)
@info "Create final ALL.ZIP"
run(`zip -9 -X $ds_dir/all.zip $ds_dir/source.zip $ds_dir/include.zip $ds_dir/jcl.zip`)

# Write the JCL files into a single alljcl.jcl file which will be used by MVS as a jobstream
@info "Preparing batch job files"
rm("$ds_dir/alljcl.jcl", force = true)
jcl_file_1 = open(f->read(f, String), "$src_dir/jcl/transfer.jcl")
jcl_file_2 = open(f->read(f, String), "$src_dir/jcl/dokernel.jcl")
jcl_file_3 = open(f->read(f, String), "$src_dir/jcl/clean.jcl")
open(f->write(f, "$jcl_file_1$jcl_file_2$jcl_file_3"), "$ds_dir/alljcl.jcl", "w")

@info "Copying the stage 1 and stage 2 bootloader"
cp("$src_dir/tools/stage1.txt", "$ds_dir/stage1.txt")
cp("$src_dir/tools/stage2.bin", "$ds_dir/stage2.bin")
touch("$ds_dir/Limine.cfg")

# MVS3.8j provides a batch file to do stuff
@info "Run MVS"
run(`cmd /c build_core.bat $mvs_prefix $ds_dir`)

@info "Copying tapes from the MVS3.8j system"
mkdir("$ds_dir/tapes")
#run(`copy $mvs_prefix/tapes/\*.\* $ds_dir/tapes/`)
for filepath in readdir("$mvs_prefix/tapes", join = true, sort = true)
    new_filename = basename(filepath)
    cp(filepath, "$ds_dir/tapes/$new_filename")
end

@info "Extracting kernel.bin from tape"
run(`hetget $ds_dir/tapes/mftopc.het $ds_dir/kernel.bin 1`)

@info "Creating uDOS system CCKD"
cd("$ds_dir")
cp("$src_dir/dasdctl.txt", "$ds_dir/dasdctl.txt")
run(`dasdload -bz2 $src_dir/dasdctl.txt sysdsk00.cckd`)
cd("$src_dir")

@info "Generating configuration file"
run(`cmd /c cnfgen.bat`)

@info "Starting hercules"
run(`hercules -f $src_dir/hercules.cnf`)

@info "Goodbye! :)"
