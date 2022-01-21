//************************************************************************
//* BUILDRTL - 20/11/2021
//* Builds the runtime library
//*
//************************************************************************
//UDRTLAS JOB CLASS=C,REGION=0K
//*
//************************************************************************
//* COMPDO
//* 
//************************************************************************
//COMPDO  PROC PREF='UDOS',MEMBER='',
// COS1='-Wall -O0 -S -ansi -DDEBUG=1',
// COS2='-DMACHINE=390 -o dd:out -'
//*
//COMP     EXEC PGM=GCC,
// PARM='&COS1 &COS2'
//STEPLIB  DD DSN=GCC.LINKLIB,DISP=SHR
//SYSIN    DD DSN=&PREF..SOURCE(&MEMBER),DISP=SHR
//INCLUDE  DD DSN=&PREF..INCLUDE,DISP=SHR,DCB=BLKSIZE=32720
//SYSINCL  DD DSN=&PREF..INCLUDE,DISP=SHR,DCB=BLKSIZE=32720
//OUT      DD DSN=&&TEMP1,DISP=(,PASS),UNIT=SYSALLDA,
//            DCB=(LRECL=80,BLKSIZE=6080,RECFM=FB),
//            SPACE=(6080,(500,500))
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//*
//ASM      EXEC PGM=ASMA90,
//            PARM='DECK,LIST',
//            COND=(4,LT,COMP)
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=PDPCLIB.MACLIB,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(4000,0)),
//            DISP=(MOD,PASS)
//SYSIN    DD DSN=&&TEMP1,DISP=(OLD,DELETE)
//*
//         PEND
//************************************************************************
//* ASMDO
//*
//************************************************************************
//ASMDO   PROC PREF='UDOS',MEMBER=''
//************************************************************************
//* Convert the asm source files from 255 VB datasets to 80-column FB
//* dataset (temporarily) for assembling without problems (asma90 can't
//* perform assembling on VB datasets for some reason)
//************************************************************************
//CONV     EXEC PGM=COPYFILE,PARM='-tt dd:in dd:out'
//IN       DD DSN=&PREF..SOURCE(&MEMBER),DISP=SHR
//OUT      DD DSN=&&FBTEMP,DISP=(,PASS),
// DCB=(RECFM=FB,LRECL=80,BLKSIZE=6144),
// SPACE=(6144,(99,99)),UNIT=SYSALLDA
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//SYSIN    DD DUMMY
//*
//ASM      EXEC PGM=ASMA90,
//            PARM='DECK,LIST'
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=PDPCLIB.MACLIB,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(4000,0)),
//            DISP=(MOD,PASS)
//SYSIN    DD DSN=&&FBTEMP,DISP=(OLD,DELETE)
//*
//         PEND
//*
//KRNLAS   EXEC ASMDO,MEMBER=KRNLAS
//*
//ALLOC    EXEC COMPDO,MEMBER=ALLOC
//DATASET  EXEC COMPDO,MEMBER=DATASET
//DEBUG    EXEC COMPDO,MEMBER=DEBUG
//ERROR    EXEC COMPDO,MEMBER=ERROR
//INTERACT EXEC COMPDO,MEMBER=INTERACT
//KRNL32   EXEC COMPDO,MEMBER=KRNL32
//MEMORY   EXEC COMPDO,MEMBER=MEMORY
//*
//LKED     EXEC PGM=IEWL,
// PARM='MAP,LIST,SIZE=(999424,65536),AMODE=31,RMODE=ANY'
//SYSUT1   DD UNIT=SYSDA,SPACE=(CYL,(30,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLMOD  DD DSN=UDOS.LINKLIB(RTL),DISP=SHR
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=UDOS.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=RTL
/*
//*
//LOADZERO EXEC PGM=LOADZERO,PARM='dd:in dd:out 1048576'
//STEPLIB  DD DSN=SYS2.LINKLIB,DISP=(OLD,PASS)
//IN       DD DSN=&&COPY,DISP=(OLD,PASS)
//OUT      DD  DSN=HERC02.ZIP,DISP=(,KEEP),UNIT=TAPE,
//         LABEL=(1,SL),VOL=SER=MFTOPC,
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=8000)
//SYSIN    DD DUMMY
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//*