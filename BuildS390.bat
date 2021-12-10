@ECHO OFF
SETLOCAL

CLS

CALL :IsInstalled 7z,"C:\Program Files\7-Zip"
CALL :IsInstalled hercules,"C:\Program Files\Hercules\Hercules 3.07 (64 Bit)"
CALL :IsInstalled gccmvs,%CD%\Toolchain
CALL :IsInstalled runmvs,%CD%\Toolchain\mvs380

RD /S /Q Distro
MKDIR Distro Distro\DosKrnl Distro\DosRtl Distro\Programs Distro\Tapes

RMDIR /S /Q DosKrnl\Arch
MKDIR DosKrnl\Arch
XCOPY /E /I DosKrnl\S390 DosKrnl\Arch
IF ERRORLEVEL 1 (
    EXIT /B 0
)

SET CFLAGS=-O2 -std=gnu99 -Itoolchain -DM_S360=360 -DM_S370=370 -DM_S380=380 -DM_S390=390 -DM_ZARCH=400 -DTARGET_S390=1 -DMACHINE=M_S390 -ffreestanding

ECHO ***************************************************************************
ECHO uDOS Module and User Runtime Library
ECHO ***************************************************************************
(FOR %%I IN (dosrtl\*.c) DO (
    ECHO %%~I

    gccmvs %CFLAGS% -Idosrtl -S %%~I -o distro\dosrtl\%%~nI.asm
    IF ERRORLEVEL 1 (
        ECHO gccmvs returned %ERRORLEVEL%
        EXIT /B 0
    )
))

(FOR %%I IN (
    DosRtl\*.S
) DO (
    ECHO %%~I
    COPY %%~I distro\DosRtl\%%~nI.asm
))

ECHO ***************************************************************************
ECHO uDOS Programs and Utilities (for userland)
ECHO ***************************************************************************
(FOR %%I IN (programs\*.c) DO (
    ECHO %%~I

    gccmvs %CFLAGS% -Iprograms -Idosrtl -S %%~I -o distro\programs\%%~nI.asm
    IF ERRORLEVEL 1 (
        ECHO gccmvs returned %ERRORLEVEL%
        EXIT /B 0
    )
))

ECHO ***************************************************************************
ECHO uDOS Platform Generic High-Level Kernel
ECHO ***************************************************************************
(FOR %%I IN (
    doskrnl\*.c
    doskrnl\comm\*.c
    doskrnl\debug\*.c
    doskrnl\fs\*.c
    doskrnl\loader\*.c
    doskrnl\mm\*.c
) DO (
    ECHO %%~I

    gccmvs %CFLAGS% -Idoskrnl -S %%~I -o distro\doskrnl\%%~nI.asm
    IF ERRORLEVEL 1 (
        ECHO gccmvs returned %ERRORLEVEL%
        EXIT /B 0
    )
))

ECHO ***************************************************************************
ECHO uDOS Platform Specific Low-Level Kernel
ECHO ***************************************************************************
(FOR %%I IN (
    doskrnl\s390\*.c
) DO (
    ECHO %%~I

    gccmvs %CFLAGS% -Idoskrnl -Idoskrnl\s390 -S %%~I -o distro\doskrnl\%%~nI.asm
    IF ERRORLEVEL 1 (
        ECHO gccmvs returned %ERRORLEVEL%
        EXIT /B 0
    )
))

(FOR %%I IN (
    doskrnl\*.S
    doskrnl\s390\*.S
) DO (
    ECHO %%~I
    COPY %%~I distro\doskrnl\%%~nI.asm
))

ECHO Zipping all source files
7z a -tzip Distro\DosKrnl.zip %CD%\Distro\DosKrnl\*.asm
7z a -tzip Distro\DosRtl.zip %CD%\Distro\DosRtl\*.asm
7z a -tzip Distro\Programs.zip %CD%\Distro\Programs\*.asm
7z a -tzip Distro\Jcl.zip %CD%\Jcl\*.jcl
7z a -tzip Distro\All.zip %CD%\Distro\DosKrnl.zip %CD%\Distro\DosRtl.zip %CD%\Distro\Programs.zip %CD%\Distro\Jcl.zip

TYPE Jcl\TransferFiles.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\AsmRtl.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\AsmUtils.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\AsmKernel.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\LoadZero.jcl >>Distro\TmpJcl.jcl
TYPE Jcl\Clean.jcl >>Distro\TmpJcl.jcl
ECHO Running MVS
CALL runmvs Distro\TmpJcl.jcl Distro\Output.txt Distro\All.zip

ECHO Extracting binary from tape
hetget "Toolchain\mvs380\tapes\mftopc.het" Distro\DosKrnl.bin 1

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