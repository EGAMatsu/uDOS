//************************************************************************
//* ASMKERNEL.JCL
//* 20/11/2021
//*
//* ASSEMBLE THE FILES THAT ARE PART OF THE KERNEL - THE LOW-LEVEL
//* PARTS OF THE KERNEL AND THE HIGH-LEVEL ONES ARE ALREADY MIXED BY
//* THE PREVIOUS GCCMVS RUN. OUR ONLY JOB IS TO ASSEMBLE SAID FILES AND
//* BIND THEM TO FORM THE FINAL EXECUTABLE.
//*
//************************************************************************
//UDKRNL  JOB CLASS=C,REGION=0K
//*
//************************************************************************
//* COMPDO
//* 
//************************************************************************
//COMPDO  PROC PREF='UDOS',MEMBER=''
//*
//COMP     EXEC PGM=GCC,
// PARM='-Wall -Wextra -O0 -S -std=gnu99 -DMACHINE=390 -ffreestanding -o dd:out -'
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
//SYSIN    DD DSN=&PREF..SOURCE(&MEMBER),DISP=SHR
//*
//         PEND
//*
//ENTRY    EXEC ASMDO,MEMBER=ENTRY
//HANDLERS EXEC ASMDO,MEMBER=HANDLERS
//CSSASM   EXEC ASMDO,MEMBER=CSSASM
//*
//INIT     EXEC COMPDO,MEMBER=INIT
//BSC      EXEC COMPDO,MEMBER=BSC
//CONTEXT  EXEC COMPDO,MEMBER=CONTEXT
//CPU      EXEC COMPDO,MEMBER=CPU
//CSS      EXEC COMPDO,MEMBER=CSS
//CRYPTO   EXEC COMPDO,MEMBER=CRYPTO
//DISASM   EXEC COMPDO,MEMBER=DISASM
//ELF      EXEC COMPDO,MEMBER=ELF
//EXT4     EXEC COMPDO,MEMBER=EXT4
//FORMAT   EXEC COMPDO,MEMBER=FORMAT
//FS       EXEC COMPDO,MEMBER=FS
//HDEBUG   EXEC COMPDO,MEMBER=HDEBUG
//INTERRUP EXEC COMPDO,MEMBER=INTERRUP
//IRQ      EXEC COMPDO,MEMBER=IRQ
//KERNEL   EXEC COMPDO,MEMBER=KERNEL
//MEMORY   EXEC COMPDO,MEMBER=MEMORY
//MM       EXEC COMPDO,MEMBER=MM
//MMU      EXEC COMPDO,MEMBER=MMU
//MUTEX    EXEC COMPDO,MEMBER=MUTEX
//PANIC    EXEC COMPDO,MEMBER=PANIC
//PE       EXEC COMPDO,MEMBER=PE
//PMM      EXEC COMPDO,MEMBER=PMM
//PRINTF   EXEC COMPDO,MEMBER=PRINTF
//REGISTRY EXEC COMPDO,MEMBER=REGISTRY
//SCHEDULE EXEC COMPDO,MEMBER=SCHEDULE
//*UBSAN    EXEC COMPDO,MEMBER=UBSAN
//USER     EXEC COMPDO,MEMBER=USER
//X3270    EXEC COMPDO,MEMBER=X3270
//X2703    EXEC COMPDO,MEMBER=X2703
//X3390    EXEC COMPDO,MEMBER=X3390
//ZDSFS    EXEC COMPDO,MEMBER=ZDSFS
//*
//LKED     EXEC PGM=IEWL,
// PARM='MAP,LIST,SIZE=(999424,65536),AMODE=31,RMODE=ANY'
//SYSUT1   DD UNIT=SYSDA,SPACE=(CYL,(30,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLMOD  DD DSN=UDOS.LINKLIB(KERNEL),DISP=SHR
//*
