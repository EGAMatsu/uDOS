//************************************************************************
//* ASMUTILS.JCL
//* 21/11/2021
//*
//* ASSEMBLE THE PROGRAMS AND UTILITIES THAT COME WITH UDOS
//*
//************************************************************************
//UDUTILS JOB CLASS=C,REGION=0K
//*
//COMPL   PROC UDPREF='UDOS',MEMBER=''
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
//SYSIN    DD DSN=&UDPREF..PROGRAMS(&MEMBER),DISP=SHR
//*
//         PEND
//*
//LINK     PROC UDPREF='UDOS',MEMBER=''
//LKED     EXEC PGM=IEWL,
// PARM='MAP,LIST,SIZE=(999424,65536),AMODE=31,RMODE=ANY'
//SYSUT1   DD UNIT=SYSDA,SPACE=(CYL,(30,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLMOD  DD DSN=&UDPREF..LINKLIB(&MEMBER),DISP=SHR
//SYSLIB   DD DSN=UDOS.LINKLIB,DISP=SHR
//         PEND
//COPYTAPE PROC UDPREF='UDOS',MEMBER=''
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=UDOS.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=HERC02.&MEMBER,DISP=(,KEEP),UNIT=TAPE,
//         LABEL=(1,SL),VOL=SER=MFTOPC,
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=8000)
//SYSPRINT DD SYSOUT=*
//         PEND
//************************************************************************
//* BRAINFUCK INTERPRETER
//*
//************************************************************************
//BFUCK    EXEC COMPL,MEMBER=BFUCK
//DOLINK   EXEC LINK,MEMBER='BFUCK'
//LKED.SYSLIN DD
//         DD *
 INCLUDE SYSLIB(RTL)
/*
//DOCOPY   EXEC COPYTAPE,MEMBER='BFUCK'
//IEBCOPY.SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=BFUCK
/*
//************************************************************************
//* COMMAND EXECUTOR
//*
//************************************************************************
//CMDEXEC  EXEC COMPL,MEMBER=CMDEXEC
//DOLINK   EXEC LINK,MEMBER='CMDEXEC'
//LKED.SYSLIN DD
//         DD *
 INCLUDE SYSLIB(RTL)
/*
//DOCOPY   EXEC COPYTAPE,MEMBER='CMDEXEC'
//IEBCOPY.SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=CMDEXEC
/*
//************************************************************************
//* COPY
//*
//************************************************************************
//COPY     EXEC COMPL,MEMBER=COPY
//DOLINK   EXEC LINK,MEMBER='COPY'
//LKED.SYSLIN DD
//         DD *
 INCLUDE SYSLIB(RTL)
/*
//DOCOPY   EXEC COPYTAPE,MEMBER='COPY'
//IEBCOPY.SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=COPY
/*
//************************************************************************
//* FORTH INTERPRETER
//*
//************************************************************************
//FORTH    EXEC COMPL,MEMBER=FORTH
//DOLINK   EXEC LINK,MEMBER='FORTH'
//LKED.SYSLIN DD
//         DD *
 INCLUDE SYSLIB(RTL)
/*
//DOCOPY   EXEC COPYTAPE,MEMBER='FORTH'
//IEBCOPY.SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=FORTH
/*
//************************************************************************
//* HELLO WORLD
//*
//************************************************************************
//HELLO    EXEC COMPL,MEMBER=HELLO
//DOLINK   EXEC LINK,MEMBER='HELLO'
//LKED.SYSLIN DD
//         DD *
 INCLUDE SYSLIB(RTL)
/*
//DOCOPY   EXEC COPYTAPE,MEMBER='HELLO'
//IEBCOPY.SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=HELLO
/*
//************************************************************************
//* NEWS READER
//*
//************************************************************************
//NEWSRDR  EXEC COMPL,MEMBER=NEWSRDR
//DOLINK   EXEC LINK,MEMBER='NEWSRDR'
//LKED.SYSLIN DD
//         DD *
 INCLUDE SYSLIB(RTL)
/*
//DOCOPY   EXEC COPYTAPE,MEMBER='NEWSRDR'
//IEBCOPY.SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=NEWSRDR
/*
//*
