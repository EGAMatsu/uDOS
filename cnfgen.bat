@echo off

rem jason gui dislikes relative paths for some reason
rem so if you plan to use that you can use this script :)

(
echo ARCHMODE  ESA/390
echo CPUSERIAL 000611
echo CPUMODEL  4381
echo NUMCPU    8
echo MAINSIZE  32
echo XPNDSIZE  0
echo CNSLPORT  3270
echo PGMPRDOS  RESTRICTED
echo CODEPAGE  819/1047
echo SYSEPOCH  1900
echo DIAG8CMD  ENABLE
echo PANOPT MSGCOLOR=DARK RATE=250

rem the windows escape character is ^
echo # .-----------------------------Device number
echo # ^|     .-----------------------Device type
echo # ^|     ^|       .---------------File name
echo # ^|     ^|       ^|
echo # V     V       V
)>"hercules.cnf"

rem system required peripherals
echo #---    ----    --------------------------------- >>"hercules.cnf"
echo 0104    1403    %CD%\distro\prt004.txt append cctape=legacy >>"hercules.cnf"
echo 01b9    3390    %CD%\distro\sysdsk00.cckd >>"hercules.cnf"
echo 0105    2703    lport=3780 dial=in term=tty >>"hercules.cnf"

set devnum=200

rem tape devices
echo #---    ----    --------------------------------- >>"hercules.cnf"
for /r %%i in (distro/tapes/*.het) do (
	echo 0%devnum%    3590    %%i >>"hercules.cnf"
	set /a devnum=%devnum%+1
)

rem disk devices
echo #---    ----    --------------------------------- >>"hercules.cnf"
for /r %%i in (distro/disks/*.cckd) do (
	echo 0%devnum%    3390    %%i >>"hercules.cnf"
	set /a devnum=%devnum%+1
)

rem time-shared terminals
for /l %%i in (400,1,410) do (
	echo 0%%i    3270 >>"hercules.cnf"
)

rem end
echo #---    ----    --------------------------------- >>"hercules.cnf"
