         CSECT
*
* CssStoreChannel
* IN:
*    schid
*    schib pointer
* OUT:
*    cc code
*
         ENTRY CSSSTORE
CSSSTORE DS 0H
         SAVE (14,12),,CSSSTORE
         LR 12,15
         USING CSSSTORE,12
         LR 11,1
         L 1,0(11)
         L 2,4(11)
         STSCH 0(2)
         ICM 15,B'0011',=X'FFFF'
*         IPM 15
         DC X'B22200F0'
         SRL 15,28
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
* CssStartChannel
* IN:
*    schid
*    schib pointer
* OUT:
*    cc code
*
         ENTRY CSSSTART
CSSSTART DS 0H
         SAVE (14,12),,CSSSTART
         LR 12,15
         USING CSSSTART,12
         LR 11,1
         L 1,0(11)
         L 2,4(11)
         SSCH 0(2)
         ICM 15,B'0011',=X'FFFF'
*         IPM 15
         DC X'B22200F0'
         SRL 15,28
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
* CssModifyChannel
* IN:
*    schid
*    schib pointer
* OUT:
*    cc code
*
         ENTRY CSSMODIF
CSSMODIF DS 0H
         SAVE (14,12),,CSSMODIF
         LR 12,15
         USING CSSMODIF,12
         LR 11,1
         L 1,0(11)
         L 2,4(11)
         MSCH 0(2)
         ICM 15,B'0011',=X'FFFF'
*         IPM 15
         DC X'B22200F0'
         SRL 15,28
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
* CssTestChannel
* IN:
*    schid
*    schib pointer
* OUT:
*    cc code
*
         ENTRY CSSTESTC
CSSTESTC DS 0H
         SAVE (14,12),,CSSTESTC
         LR 12,15
         USING CSSTESTC,12
         LR 11,1
         L 1,0(11)
         L 2,4(11)
         MSCH 0(2)
         ICM 15,B'0011',=X'FFFF'
*         IPM 15
         DC X'B22200F0'
         SRL 15,28
         RETURN (14,12),RC=(15)
         LTORG
         DROP 12
*
         END