***********************************************************************
*
* ESA 390 boot code, this entry code is independent from z/Arch or 360
* it should run on whatever machine flawlessly
*
***********************************************************************
         CSECT
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
         EXTRN KEINIT
         ENTRY START
START    DS 0H
         BALR 12,0
         USING *,12
* Set the stack pointer
         L 13,=A(@@STACK)
         LA 5,180(13)
         ST 5,76(13)
* Enable I/O interrupts
         LCTL 6,6,ALLIOINT
         LPSW WAITER1
AFTERRLD DS 0H
* Jump to the kernel C entry - goodbye HLASM!
         L 15,=V(KEINIT)
         BR 15
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
         LR 12,15
         USING @@DIAG8,12
         LR 11,1
         L 1,0(11)
         L 2,4(11)
*         DIAG 1,2,8
         DC X'83120008'
         L 15,=F'0'
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
         LR 12,15
         USING @ZHWCTID,12
         LR 11,1
         L 15,0(1)
* Load the current timer into TIMETMP, then load it onto R1
         STPT TIMETMP
         L 1,=A(TIMETMP)
* Save the delta into the CPU timer
         ST 15,TIMETMP
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
         LR 12,15
         USING @ZHWPUID,12
         LR 11,1
         L 1,=A(PGT0)
*         STAP 1
         DC X'B2121000'
         L 15,0(1)
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
         LR 12,15
         USING @ZHWSIGP,12
         LR 11,1
* R1 = status
* R2 = cpuid
* R3 = parameter
         L 1,=X'00000000'
         L 2,0(11)
         L 3,4(11)
         SIGP 1,2,3
         ICM 15,B'0011',=X'FFFF'
*         IPM 15
         DC X'B22200F0'
         SRL 15,28
         L 15,0(1)
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
         LR 12,15
         USING @ZHWDSVC,12
         LR 11,1
         L 4,0(11)
         L 1,4(11)
         L 2,8(11)
         L 3,12(11)
         SVC 26
         L 15,4
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
* Our stack
@@STACK  DS 1024F
         END