@ECHO OFF
SETLOCAL

CLS

CALL :IsInstalled 7z,"C:\Program Files\7-Zip"
CALL :IsInstalled hercules,"C:\Program Files\Hercules\Hercules 3.07 (64 Bit)"
CALL :IsInstalled runmvs,%CD%\toolchain\mvs380

RMDIR /S /Q distro
MKDIR distro distro\tapes

ECHO Zipping all source files
7z a -tzip distro\source.zip %CD%\kernel\*.asm %CD%\kernel\*.c
7z a -tzip distro\include.zip %CD%\kernel\*.h
7z a -tzip distro\jcl.zip %CD%\jcl\*.jcl
7z a -tzip distro\All.zip %CD%\distro\source.zip %CD%\distro\include.zip %CD%\distro\jcl.zip

TYPE Jcl\transfer.jcl >>distro\alljcl.jcl
TYPE Jcl\compile.jcl >>distro\alljcl.jcl
TYPE Jcl\loadzero.jcl >>distro\alljcl.jcl
TYPE Jcl\clean.jcl >>distro\alljcl.jcl
ECHO Running MVS
CALL runmvs distro\alljcl.jcl distro\output.txt distro\all.zip

ECHO Extracting binary from tape
hetget "Toolchain\mvs380\tapes\mftopc.het" distro\kernel.bin 1

ECHO Creating final disk
COPY tools\stage1.txt distro\stage1.txt
COPY tools\stage2.bin distro\stage2.bin
ECHO #File >distro\Limine.cfg
COPY dasdctl.txt distro\dasdctl.txt

CD distro
DEL sysdsk00.cckd
dasdload -bz2 ..\dasdctl.txt sysdsk00.cckd
CD ..

COPY toolchain\mvs380\tapes\*.het distro\tapes\

hercules -f hercules.cnf >distro\log.txt
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