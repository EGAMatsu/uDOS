         COPY PDPTOP
         PRINT GEN
         YREGS
*
         AIF ('&ZSYS' EQ 'S370').AMB24A
AMBIT    EQU X'80000000'
         AGO .AMB24B
.AMB24A  ANOP
AMBIT    EQU X'00000000'
.AMB24B  ANOP
*
*         AIF ('&ZAM64' NE 'YES').AMZB24A
*AM64BIT  EQU X'00000001'
*         AGO .AMZB24B
*.AMZB24A ANOP
*AM64BIT  EQU X'00000000'
*.AMZB24B ANOP
*
         CSECT
* Register save areas
FLCGRSAV EQU   384   A(X'180')
FLCCRSAV EQU   448   A(X'1C0')
* New PSWs
FLCPNPSW EQU   104   A(X'68')
* Old PSWs
FLCEOPSW EQU   24    A(X'18')
FLCSOPSW EQU   32    A(X'20')
FLCPOPSW EQU   40    A(X'28')
FLCMOPSW EQU   48    A(X'30')
FLCIOPSW EQU   56    A(X'38')
* Service call handler
*
         EXTRN @ZHSVC
         ENTRY @ASVCHDL
@ASVCHDL DS 0H
         STM R0,R15,FLCGRSAV
         LM R0,R15,FLCCRSAV
         BALR R12,R0
         USING *,12
         L R13,=A(@FLSTACK)
         LA R5,180(R13)
         ST R5,76(R13)
*
         L R15,=V(@ZHSVC)
         BALR R14,R15
*
         LM R0,R15,FLCGRSAV
         LPSW FLCSOPSW
         LTORG
         DROP 12
* Program check handler
*
         EXTRN @ZHPC
         ENTRY @APCHDL
@APCHDL  DS 0H
         STM R0,R15,FLCGRSAV
         LM R0,R15,FLCCRSAV
         BALR R12,R0
         USING *,12
         L R13,=A(@FLSTACK)
         LA R5,180(R13)
         ST R5,76(R13)
*
         L R15,=V(@ZHPC)
         BALR R14,R15
*
         LM R0,R15,FLCGRSAV
         LPSW FLCPOPSW
         LTORG
         DROP 12
* External interruption handler
*
         EXTRN @ZHEXT
         ENTRY @AEXTHDL
@AEXTHDL DS 0H
         STM R0,R15,FLCGRSAV
         LM R0,R15,FLCCRSAV
         BALR R12,R0
         USING *,12
         L R13,=A(@FLSTACK)
         LA R5,180(R13)
         ST R5,76(R13)
*
         L R15,=V(@ZHEXT)
         BALR R14,R15
*
         LM R0,R15,FLCGRSAV
         LPSW FLCEOPSW
         LTORG
         DROP 12
* Machine check handler
*
         EXTRN @ZHMC
         ENTRY @AMCHDL
@AMCHDL  DS 0H
         STM R0,R15,FLCGRSAV
         LM R0,R15,FLCCRSAV
         BALR R12,R0
         USING *,12
         L R13,=A(@FLSTACK)
         LA R5,180(R13)
         ST R5,76(R13)
*
         L R15,=V(@ZHMC)
         BALR R14,R15
*
         LM R0,R15,FLCGRSAV
         LPSW FLCMOPSW
         LTORG
         DROP 12
* I/O interruption handler
*
         EXTRN @ZHIO
         ENTRY @AIOHDL
@AIOHDL  DS 0H
         STM R0,R15,FLCGRSAV
         LM R0,R15,FLCCRSAV
         BALR R12,R0
         USING *,12
         L R13,=A(@FLSTACK)
         LA R5,180(R13)
         ST R5,76(R13)
*
         L R15,=V(@ZHIO)
         BALR R14,R15
*
         LM R0,R15,FLCGRSAV
         LPSW FLCIOPSW
         LTORG
         DROP 12
* HwCheckAddress
* IN:
*    pointer to address to probe
*
         ENTRY @ZHWCHKA
@ZHWCHKA DS 0H
         SAVE (14,12),,@ZHWCHKA
         LR R12,R15
         USING @ZHWCHKA,12
         LR R11,R1
* CATCHPSW address on R1
         L R1,=A(CATCHPSW)
* Save address of TMPSAVE on R2
         L R2,=A(TMPSAVE)
* And FLCPNPSW on R3
         L R3,FLCPNPSW
*
         MVC 0(8,R2),0(R3)
* ... use a new PSW to catch the exceptions
         MVC 0(8,R3),0(R1)
* Probe the address, if it raises a PC exception then
* we will simply catch it and return 1
         L R1,0(R11)
         L R15,0(R1)
* rc = 0
         MVC 0(8,R3),0(R2)
         L R15,=F'0'
         RETURN (14,12),RC=(15)
CATCHPCR DS 0H
* rc = 1
         MVC 0(8,R3),0(R2)
         L R15,=F'1'
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
CATCHPSW DS 0D
         DC X'020E0000'
         DC A(AMBIT+CATCHPCR)
TMPSAVE  DS 1D
*
@FLSTACK DS 1024F
*
         END