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
//CLEAN    PROC PREF='UDOS'
//DELETE   EXEC PGM=IEFBR14
//DD1      DD DSN=&PREF..SOURCE,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD2      DD DSN=&PREF..INCLUDE,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD4      DD DSN=&PREF..JCL,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD5      DD DSN=&PREF..LINKLIB,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//DD6      DD DSN=&PREF..ALLZIPS,DISP=(MOD,DELETE),
//         UNIT=SYSDA,SPACE=(TRK,(0))
//         PEND
/*
//S2       EXEC CLEAN
//
