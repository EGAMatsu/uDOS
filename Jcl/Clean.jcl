//************************************************************************
//* CLEAN.JCL
//* 20/11/2021
//*
//* CLEAN THE LEFTOVERS AFTER DOING THE COMPILATION TO ALLOW FOR MULTIPLE
//* PASSES WITHOUT PROBLEMS
//*
//************************************************************************
//UDCLEAR  JOB CLASS=C,REGION=0K
//*
//CLEAN    PROC UDPREF='UDOS'
//DELETE   EXEC PGM=IEFBR14
//DD1      DD DSN=&UDPREF..DOSKRNL,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD2      DD DSN=&UDPREF..DOSRTL,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD3      DD DSN=&UDPREF..PROGRAMS,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD4      DD DSN=&UDPREF..JCL,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD5      DD DSN=&UDPREF..LINKLIB,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD6      DD DSN=&UDPREF..ALLZIPS,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//         PEND
/*
//S2       EXEC CLEAN
//
