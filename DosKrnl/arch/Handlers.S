         CSECT
* Register save areas
FLCGRSAV EQU   384   A(X'180')
FLCCRSAV EQU   448   A(X'1C0')
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
         STM 0,15,FLCGRSAV
         LM 0,15,FLCCRSAV
         BALR 12,0
         USING *,12
         L 15,=V(@ZHIO)
         BALR 14,15
         LM 0,15,FLCGRSAV
         LPSW FLCIOPSW
         LTORG
         DROP 12
*
         END