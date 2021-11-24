         CSECT
* HwDoSVC
* IN:
*    arg0
*    arg1
*    arg2
*    svc code
* OUT:
*    retcode
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
*
         END