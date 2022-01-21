setlocal
cls

rmdir /S /Q distro
mkdir distro distro\tapes

rem Build the uDOS kernel
zip -9 -X -ll distro\source.zip %CD%\kernel\*.asm %CD%\kernel\*.c
zip -9 -X -ll distro\include.zip %CD%\kernel\*.h
zip -9 -X -ll distro\jcl.zip %CD%\jcl\*.jcl
zip -9 -X distro\all.zip %CD%\distro\source.zip %CD%\distro\include.zip %CD%\distro\jcl.zip
del distro\alljcl.jcl
type jcl\transfer.jcl >>distro\alljcl.jcl
type jcl\dokernel.jcl >>distro\alljcl.jcl
type jcl\clean.jcl >>distro\alljcl.jcl
call c:\pdos\mvs380\runmvs distro\alljcl.jcl distro\output.txt distro\all.zip
hetget "c:\pdos\mvs380\tapes\mftopc.het" distro\kernel.bin 1

echo Creating final disk
copy tools\stage1.txt distro\stage1.txt
copy tools\stage2.bin distro\stage2.bin
echo #File >distro\Limine.cfg
copy dasdctl.txt distro\dasdctl.txt

cd distro
del sysdsk00.cckd
dasdload -bz2 ..\dasdctl.txt sysdsk00.cckd
cd ..

copy c:\pdos\mvs380\tapes\*.het distro\tapes\

hercules -f hercules.cnf >distro\log.txt
exit /B 0