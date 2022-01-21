@ECHO OFF
SETLOCAL

SET "PATH=%PATH%;C:\Program Files (x86)\GnuWin32\bin"
WHERE /q enscript
IF ERRORLEVEL 1 (
    ECHO enscript is not installed - please install it!
    EXIT /B 0
)

SET "PATH=%PATH%;C:\Program Files\gs\gs9.55.0\bin"
WHERE /q gswin64c
IF ERRORLEVEL 1 (
    ECHO gswin64c is not installed - please install it!
    EXIT /B 0
)

enscript --font=Courier-Bold@8 -l -H1 -r --margins=25:25:40:40 -p distro/log.ps distro/output.txt
gswin64c -sPAPERSIZE#legal -sDEVICE#pdfwrite -o distro/log.pdf distro/log.ps

EXIT /B 0
