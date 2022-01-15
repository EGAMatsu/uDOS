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
         STM 0,15,FLCGRSAV
         LM 0,15,FLCCRSAV
         BALR 12,0
         USING *,12
         L 15,=V(@ZHSVC)
         BALR 14,15
         LM 0,15,FLCGRSAV
         LPSW FLCSOPSW
         LTORG
         DROP 12
* Program check handler
*
         EXTRN @ZHPC
         ENTRY @APCHDL
@APCHDL  DS 0H
         STM 0,15,FLCGRSAV
         LM 0,15,FLCCRSAV
         BALR 12,0
         USING *,12
         L 15,=V(@ZHPC)
         BALR 14,15
         LM 0,15,FLCGRSAV
         LPSW FLCPOPSW
         LTORG
         DROP 12
* External interruption handler
*
         EXTRN @ZHEXT
         ENTRY @AEXTHDL
@AEXTHDL DS 0H
         STM 0,15,FLCGRSAV
         LM 0,15,FLCCRSAV
         BALR 12,0
         USING *,12
         L 15,=V(@ZHEXT)
         BALR 14,15
         LM 0,15,FLCGRSAV
         LPSW FLCEOPSW
         LTORG
         DROP 12
* Machine check handler
*
         EXTRN @ZHMC
         ENTRY @AMCHDL
@AMCHDL  DS 0H
         STM 0,15,FLCGRSAV
         LM 0,15,FLCCRSAV
         BALR 12,0
         USING *,12
         L 15,=V(@ZHMC)
         BALR 14,15
         LM 0,15,FLCGRSAV
         LPSW FLCMOPSW
         LTORG
         DROP 12
* I/O interruption handler
*
         EXTRN @ZHIO
         ENTRY @AIOHDL
@AIOHDL  DS 0H
         BALR 12,0
         STM 0,15,FLCGRSAV
         LM 0,15,FLCCRSAV
         USING *,12
         L 15,=V(@ZHIO)
         BALR 14,15
         LM 0,15,FLCGRSAV
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
         LR 12,15
         USING @ZHWCHKA,12
         LR 11,1
* CATCHPSW address on R1
         L 1,=A(CATCHPSW)
* Save address of TMPSAVE on R2
         L 2,=A(TMPSAVE)
* And FLCPNPSW on R3
         L 3,FLCPNPSW
*
         MVC 0(8,2),0(3)
* ... use a new PSW to catch the exceptions
         MVC 0(8,3),0(1)
* Probe the address, if it raises a PC exception then
* we will simply catch it and return 1
         L 1,0(11)
         L 15,0(1)
* rc = 0
         MVC 0(8,3),0(2)
         L 15,=F'0'
         RETURN (14,12),RC=(15)
CATCHPCR DS 0H
* rc = 1
         MVC 0(8,3),0(2)
         L 15,=F'1'
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
CATCHPSW DS 0D
         DC X'020E0000'
         DC A(AMBIT+CATCHPCR)
TMPSAVE  DS 1D
*
         END