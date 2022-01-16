***********************************************************************
*
* ESA 390 boot code, this entry code is independent from z/Arch or 360
* it should run on whatever machine flawlessly
*
***********************************************************************
         COPY PDPTOP
         PRINT GEN
         YREGS
*
*         AIF ('&ZSYS' EQ 'S370').AMB24A
AMBIT    EQU X'80000000'
*         AGO .AMB24B
*.AMB24A  ANOP
*AMBIT    EQU X'00000000'
*.AMB24B  ANOP
*         AIF ('&ZAM64' NE 'YES').AMZB24A
*AM64BIT  EQU X'00000001'
*         AGO .AMZB24B
*.AMZB24A ANOP
*AM64BIT  EQU X'00000000'
*.AMZB24B ANOP
*
         CSECT
*
         EXTRN KEINIT
         ENTRY START
START    DS 0H
         BALR R12,0
         USING *,12
* Set the stack pointer
         L R13,=A(@@STACK)
         LA R5,180(R13)
         ST R5,76(R13)
* Set backchain to zero (to help unwinding the stack)
         L R5,0
         ST R5,4(R13)
         ST R5,8(R13)
* Enable I/O interrupts
         LCTL R6,R6,ALLIOINT
         LPSW WAITER1
AFTERRLD DS 0H
* Jump to the kernel C entry - goodbye HLASM!
         L R15,=V(KEINIT)
         BR R15
         LTORG
         DROP 12
WAITER1  DS 0D
         DC X'030C0000'
         DC A(AMBIT+AFTERRLD)
* Enable all interruptions on the CPU
ALLIOINT DS 0F
         DC X'FF000000'
*
* Diag 8
* IN:
*    pointer to ebcdic message
*    length
*
         ENTRY @@DIAG8
@@DIAG8  DS 0H
         SAVE (14,12),,@@DIAG8
         LR R12,R15
         USING @@DIAG8,12
         LR R11,R1
         L R1,0(R11)
         L R2,4(R11)
*         DIAG 1,2,8
         DC X'83120008'
         L R15,=F'0'
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
* HwSetCPUTimerDelta
* IN:
*    time_delta
*
         ENTRY @ZHWCTID
@ZHWCTID DS 0H
         SAVE (14,12),,@ZHWCTID
         LR R12,R15
         USING @ZHWCTID,12
         LR R11,R1
         L R15,0(R1)
* Load the current timer into TIMETMP, then load it onto R1
         STPT TIMETMP
         L R1,=A(TIMETMP)
* Save the delta into the CPU timer
         ST R15,TIMETMP
         SPT =A(TIMETMP)
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
TIMETMP  DS 1D
*
* HwCPUID
* OUT:
*    current cpuid
*
         ENTRY @ZHWPUID
@ZHWPUID DS 0H
         SAVE (14,12),,@ZHWPUID
         LR R12,R15
         USING @ZHWPUID,12
         LR R11,R1
         L R1,=A(PGT0)
*         STAP R1
         DC X'B2121000'
         L R15,0(R1)
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
PGT0     DS 1F
*
* HwSignalCPU
* IN:
*    cpuid
*    signal code
* OUT:
*    cc code
*
         ENTRY @ZHWSIGP
@ZHWSIGP DS 0H
         SAVE (14,12),,@ZHWSIGP
         LR R12,R15
         USING @ZHWSIGP,12
         LR R11,R1
* R1 = status
* R2 = cpuid
* R3 = parameter
         L R1,=X'00000000'
         L R2,0(R11)
         L R3,4(R11)
         SIGP R1,R2,R3
         ICM R15,B'0011',=X'FFFF'
*         IPM R15
         DC X'B22200F0'
         SRL R15,28
         L R15,0(R1)
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
* HwDoSVC
* IN:
*    arg0
*    arg1
*    arg2
*    svc code
* OUT:
*    retcode
*
         ENTRY @ZHWDSVC
@ZHWDSVC DS 0H
         SAVE (14,12),,@ZHWDSVC
         LR R12,R15
         USING @ZHWDSVC,12
         LR R11,R1
         L R4,0(R11)
         L R1,4(R11)
         L R2,8(R11)
         L R3,12(R11)
         SVC 26
         LR R15,R4
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
* Our stack
@@STACK  DS 1024F
*
         END