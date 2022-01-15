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
//COMPL   PROC UDPREF='UDOS',MEMBER=''
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
//SYSIN    DD DSN=&UDPREF..DOSKRNL(&MEMBER),DISP=SHR
//*
//         PEND
//*
//LINK     PROC UDPREF='UDOS'
//LKED     EXEC PGM=IEWL,
// PARM='MAP,LIST,SIZE=(999424,65536),AMODE=31,RMODE=ANY'
//SYSUT1   DD UNIT=SYSDA,SPACE=(CYL,(30,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLMOD  DD DSN=&UDPREF..LINKLIB(KRNL),DISP=SHR
//         PEND
//*
//ENTRY    EXEC COMPL,MEMBER=ENTRY
//INIT     EXEC COMPL,MEMBER=INIT
//HANDLERS EXEC COMPL,MEMBER=HANDLERS
//BSC      EXEC COMPL,MEMBER=BSC
//CONTEXT  EXEC COMPL,MEMBER=CONTEXT
//CPU      EXEC COMPL,MEMBER=CPU
//CSSASM   EXEC COMPL,MEMBER=CSSASM
//CSS      EXEC COMPL,MEMBER=CSS
//CRYPTO   EXEC COMPL,MEMBER=CRYPTO
//DISASM   EXEC COMPL,MEMBER=DISASM
//ELF      EXEC COMPL,MEMBER=ELF
//EXT4     EXEC COMPL,MEMBER=EXT4
//FORMAT   EXEC COMPL,MEMBER=FORMAT
//FS       EXEC COMPL,MEMBER=FS
//HDEBUG   EXEC COMPL,MEMBER=HDEBUG
//INTERRUP EXEC COMPL,MEMBER=INTERRUP
//IRQ      EXEC COMPL,MEMBER=IRQ
//KERNEL   EXEC COMPL,MEMBER=KERNEL
//MEMORY   EXEC COMPL,MEMBER=MEMORY
//MM       EXEC COMPL,MEMBER=MM
//MMU      EXEC COMPL,MEMBER=MMU
//MUTEX    EXEC COMPL,MEMBER=MUTEX
//OBJ      EXEC COMPL,MEMBER=OBJ
//PANIC    EXEC COMPL,MEMBER=PANIC
//PE       EXEC COMPL,MEMBER=PE
//PMM      EXEC COMPL,MEMBER=PMM
//PRINTF   EXEC COMPL,MEMBER=PRINTF
//REGISTRY EXEC COMPL,MEMBER=REGISTRY
//SCHEDULE EXEC COMPL,MEMBER=SCHEDULE
//*UBSAN    EXEC COMPL,MEMBER=UBSAN
//USER     EXEC COMPL,MEMBER=USER
//X3270    EXEC COMPL,MEMBER=X3270
//X2703    EXEC COMPL,MEMBER=X2703
//X3390    EXEC COMPL,MEMBER=X3390
//ZDSFS    EXEC COMPL,MEMBER=ZDSFS
//*
//DOLINK   EXEC LINK
//*