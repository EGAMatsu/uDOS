@ECHO OFF
SETLOCAL

CLS

CALL :IsInstalled 7z,"C:\Program Files\7-Zip"
CALL :IsInstalled hercules,"C:\Program Files\Hercules\Hercules 3.07 (64 Bit)"
CALL :IsInstalled gccmvs,%CD%\Toolchain
CALL :IsInstalled runmvs,%CD%\Toolchain\mvs380

RMDIR /S /Q Distro
MKDIR Distro Distro\kernel Distro\rtl Distro\Programs Distro\Tapes

SET CFLAGS=-O2 -std=gnu99 -Itoolchain -DMACHINE=390 -ffreestanding

ECHO ***************************************************************************
ECHO uDOS Module and User Runtime Library
ECHO ***************************************************************************
(FOR %%I IN (rtl\*.c) DO (
    ECHO %%~I

    gccmvs %CFLAGS% -Irtl -S %%~I -o distro\rtl\%%~nI.asm
    IF ERRORLEVEL 1 (
        ECHO gccmvs returned %ERRORLEVEL%
        EXIT /B 0
    )
))

(FOR %%I IN (
    rtl\*.asm
) DO (
    ECHO %%~I
    COPY %%~I distro\rtl\%%~nI.asm
))

ECHO ***************************************************************************
ECHO uDOS Programs and Utilities (for userland)
ECHO ***************************************************************************
(FOR %%I IN (programs\*.c) DO (
    ECHO %%~I

    gccmvs %CFLAGS% -Iprograms -Irtl -S %%~I -o distro\programs\%%~nI.asm
    IF ERRORLEVEL 1 (
        ECHO gccmvs returned %ERRORLEVEL%
        EXIT /B 0
    )
))

ECHO ***************************************************************************
ECHO uDOS Platform Generic High-Level Kernel
ECHO ***************************************************************************
(FOR %%I IN (
    kernel\*.c
    kernel\comm\*.c
    kernel\debug\*.c
    kernel\fs\*.c
    kernel\loader\*.c
    kernel\mm\*.c
) DO (
    ECHO %%~I

    gccmvs %CFLAGS% -Ikernel -S %%~I -o distro\kernel\%%~nI.asm
    IF ERRORLEVEL 1 (
        ECHO gccmvs returned %ERRORLEVEL%
        EXIT /B 0
    )
))

(FOR %%I IN (
    kernel\*.asm
) DO (
    ECHO %%~I
    COPY %%~I distro\kernel\%%~nI.asm
))

ECHO Zipping all source files
7z a -tzip Distro\kernel.zip %CD%\Distro\kernel\*.asm
7z a -tzip Distro\rtl.zip %CD%\Distro\rtl\*.asm
7z a -tzip Distro\Programs.zip %CD%\Distro\Programs\*.asm
7z a -tzip Distro\Jcl.zip %CD%\Jcl\*.jcl
7z a -tzip Distro\All.zip %CD%\Distro\kernel.zip %CD%\Distro\rtl.zip %CD%\Distro\Programs.zip %CD%\Distro\Jcl.zip

TYPE Jcl\TransferFiles.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\AsmRtl.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\AsmUtils.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\AsmKernel.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\LoadZero.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\Clean.jcl >>Distro\TmpJcl.jcl
ECHO Running MVS
CALL runmvs Distro\TmpJcl.jcl Distro\Output.txt Distro\All.zip

ECHO Extracting binary from tape
hetget "Toolchain\mvs380\tapes\mftopc.het" Distro\kernel.bin 1

ECHO Creating final disk
COPY Tools\stage1.txt Distro\stage1.txt
COPY Tools\stage2.bin Distro\stage2.bin
ECHO #File >Distro\Limine.cfg
COPY DasdCtl.txt Distro\DasdCtl.txt

CD Distro
DEL SYSDSK00.cckd
dasdload -bz2 ..\DasdCtl.txt SYSDSK00.cckd
CD ..

COPY Toolchain\mvs380\tapes\*.het Distro\Tapes\

hercules -f hercules.cnf >Distro\Log.txt
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