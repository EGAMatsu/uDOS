@ECHO OFF
SETLOCAL

CLS

RMDIR /S /Q Distro
MKDIR Distro Distro\DosKrnl Distro\DosRtl Distro\Programs Distro\Tapes Distro\IPL

RMDIR /S /Q DosKrnl\Arch
MKDIR DosKrnl\Arch
XCOPY /E /I DosKrnl\x86 DosKrnl\Arch
IF ERRORLEVEL 1 (
    EXIT /B 0
)

SET TARGET_FLAGS=-DM_I386=386 -DM_I486=486 -DM_I586=586 -DM_I686=686 -DM_AMD64=786 -DTARGET_X86=1 -DMACHINE=M_I386
SET CFLAGS=%TARGET_FLAGS% -O2 -std=gnu99 -IToolchain -ffreestanding -fno-stack-protector -fno-pic
SET ASFLAGS=%TARGET_FLAGS%
SET LDFLAGS=-nostdlib -nodefaultlibs -nostartfiles -ffreestanding -fPIE -fno-pic

ECHO ***************************************************************************
ECHO uDOS Module and User Runtime Library
ECHO ***************************************************************************
(FOR %%I IN (dosrtl\*.c) DO (
    ECHO %%~I
    gcc %CFLAGS% -IDosRtl -c %%~I -o Distro\DosRtl\%%~nI.obj
    IF ERRORLEVEL 1 (
        ECHO gcc returned %ERRORLEVEL%
        EXIT /B 0
    )
))

ECHO ***************************************************************************
ECHO uDOS Programs and Utilities (for userland)
ECHO ***************************************************************************
(FOR %%I IN (programs\*.c) DO (
    ECHO %%~I
    gcc %CFLAGS% -IPrograms -IDosRtl -c %%~I -o Distro\Programs\%%~nI.obj
    IF ERRORLEVEL 1 (
        ECHO gcc returned %ERRORLEVEL%
        EXIT /B 0
    )
))

ECHO ***************************************************************************
ECHO uDOS Platform generic high-level kernel                                    
ECHO ***************************************************************************
(FOR %%I IN (
    DosKrnl\*.c
    DosKrnl\Comm\*.c
    DosKrnl\Debug\*.c
    DosKrnl\Fs\*.c
    DosKrnl\Iodev\*.c
    DosKrnl\Loader\*.c
    DosKrnl\Mm\*.c
) DO (
    ECHO %%~I
    gcc %CFLAGS% -IDosKrnl -c %%~I -o Distro\DosKrnl\%%~nI.obj
    IF ERRORLEVEL 1 (
        ECHO gcc returned %ERRORLEVEL%
        EXIT /B 0
    )
))

ECHO ***************************************************************************
ECHO uDOS Platform specific kernel                                              
ECHO ***************************************************************************
(FOR %%I IN (
    DosKrnl\x86\*.c
) DO (
    ECHO %%~I
    gcc %CFLAGS% -IDosKrnl -IDosKrnl\x86 -c %%~I -o Distro\DosKrnl\%%~nI.obj
    IF ERRORLEVEL 1 (
        ECHO gcc returned %ERRORLEVEL%
        EXIT /B 0
    )
))

(FOR %%I IN (
    DosKrnl\x86\*.S
) DO (
    ECHO %%~I
    gcc %ASFLAGS% -c %%~I -o Distro\DosKrnl\%%~nI.asm.obj
    IF ERRORLEVEL 1 (
        ECHO gcc returned %ERRORLEVEL%
        EXIT /B 0
    )
))

SETLOCAL EnableDelayedExpansion
REM Add all obj files to the LD_FILES list
SET "OBJFILES="
(FOR %%I IN (
    Distro\DosKrnl\*.obj
) DO (
    ECHO %%~I
    SET "OBJFILES=!OBJFILES! %%~I"
))
gcc -nostdlib -nodefaultlibs -nostartfiles -ffreestanding -fPIE -fno-pic -TDosKrnl\Platform\x86-pc.ld %OBJFILES% -o Distro\Kernel32.pe
objcopy -O binary Distro\Kernel32.pe Distro\Kernel32.bin

ECHO ***************************************************************************
ECHO uDOS Platform Entry Bootloader                                              
ECHO ***************************************************************************
(FOR %%I IN (
    IPL\*.S
) DO (
    ECHO %%~I
    gcc %ASFLAGS% -c %%~I -o Distro\IPL\%%~nI.asm.obj
    IF ERRORLEVEL 1 (
        ECHO gcc returned %ERRORLEVEL%
        EXIT /B 0
    )
))

SET "OBJFILES="
(FOR %%I IN (
    Distro\IPL\*.obj
) DO (
    ECHO %%~I
    SET "OBJFILES=!OBJFILES! %%~I"
))
gcc %LDFLAGS% -Ttext=0x7c00 %OBJFILES% -o Distro\Boot32.pe
objcopy -O binary Distro\Boot32.pe Distro\Boot32.bin

EXIT /B 0

REM Function to check for installed program
:IsInstalled
ECHO Checking for %~1...
WHERE /q %~1
IF ERRORLEVEL 1 (
    ECHO Adding %~1 to the path
    SET "PATH=%PATH%;%~2"
    ECHO "%PATH%"
    WHERE /q %~1
    IF ERRORLEVEL 1 (
        ECHO %~1 is not installed - please install it!
        CALL :CleanExit %ERRORLEVEL%
    )
)
EXIT /B %ERRORLEVEL%

:CleanExit
() 2>nul
SET _errLevel=%~1
:PopStack
    (goto) 2>nul
    SETLOCAL DisableDelayedExpansion    
    CALL SET "caller=%%~0"
    CALL SET _caller=%%caller:~0,1%%
    CALL SET _caller=%%_caller::=%%
    IF NOT DEFINED _caller (
        GOTO :PopStack
    )   
    (goto) 2>nul
    ENDLOCAL
    CMD /C "EXIT /B %_errLevel%"
)
ECHO Failure to clean exit!
EXIT /B