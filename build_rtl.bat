setlocal
cls

rmdir /S /Q distro
mkdir distro distro\tapes

rem Build the uDOS Runtime library
zip -9 -X -ll distro\source.zip %CD%\rtl\*.asm %CD%\rtl\*.c
zip -9 -X -ll distro\include.zip %CD%\rtl\*.h
zip -9 -X -ll distro\jcl.zip %CD%\jcl\*.jcl
zip -9 -X distro\all.zip %CD%\distro\source.zip %CD%\distro\include.zip %CD%\distro\jcl.zip
del distro\alljcl.jcl
type jcl\transfer.jcl >>distro\alljcl.jcl
type jcl\dortl.jcl >>distro\alljcl.jcl
type jcl\clean.jcl >>distro\alljcl.jcl
call c:\pdos\mvs380\runmvs distro\alljcl.jcl distro\output.txt distro\all.zip
hetget "c:\pdos\mvs380\tapes\mftopc.het" distro\rtl.lib 1
exit /B 0
