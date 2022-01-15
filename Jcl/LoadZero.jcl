//************************************************************************
//* LOADZERO.JCL
//* 20/11/2021
//*
//* COMPILE THE LOADZERO PROGRAM IN A TEMPORAL EXECUTABLE FILE THAT WILL
//* CONVERT OUR MVS MODULE INTO A FLAT BINARY WE CAN USE IN BAREMETAL
//*
//************************************************************************
//LZGEN     JOB CLASS=C,REGION=0K
//*
//* This example is for people who don't have a PROC installed
//* in their installation. It compiles an example C program.
//*
//CCOMP    PROC GCCPREF='GCC',PDPPREF='PDPCLIB',MEMBER='',
// COS1='-Os -S -ansi -pedantic-errors',
// COS2='-o dd:out -'
//*
//COMP     EXEC PGM=GCC,
// PARM='&COS1 &COS2'
//STEPLIB  DD DSN=&GCCPREF..LINKLIB,DISP=SHR
//INCLUDE  DD DSN=&PDPPREF..INCLUDE,DISP=SHR
//SYSINCL  DD DSN=&PDPPREF..INCLUDE,DISP=SHR
//OUT      DD DSN=&&TEMP,DISP=(,PASS),UNIT=SYSALLDA,
//            DCB=(LRECL=80,BLKSIZE=6160,RECFM=FB),
//            SPACE=(6160,(500,500))
//SYSIN    DD DSN=&PDPPREF..SOURCE(&MEMBER),DISP=SHR
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//*
//ASM      EXEC PGM=ASMA90,
//            PARM='DECK,NOLIST',
//            COND=(4,LT,COMP)
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=&PDPPREF..MACLIB,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(4000,4000)),
//            DISP=(,PASS)
//SYSIN    DD DSN=&&TEMP,DISP=(OLD,DELETE)
//*
//LKED     EXEC PGM=IEWL,PARM='AMODE=31,RMODE=ANY',
//            COND=((4,LT,COMP),(4,LT,ASM))
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//         DD DDNAME=SYSIN
//SYSIN    DD DUMMY
//SYSLIB   DD DSN=&PDPPREF..NCALIB,DISP=SHR
//SYSLMOD  DD DSN=&&TEMPL(&MEMBER),DISP=(OLD,PASS)
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//         PEND
//*
//S1       EXEC PGM=IEFBR14
//DD1      DD DSN=&&TEMPL,DISP=(NEW,PASS),UNIT=SYSALLDA,
//         SPACE=(CYL,(1,1,44)),DCB=(RECFM=U,LRECL=0,BLKSIZE=6144)
//*
//S2       EXEC CCOMP,MEMBER='LOADZERO'
//COMP.SYSIN DD DATA,DLM=XX
/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  loadzero - load a module into storage as if it was loaded at     */
/*  location 0.                                                      */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAXEXE 5000000

static int fixPE(char *buf, int *len, int *entry, int rlad);
static int processRLD(char *buf, int rlad, char *rld, int len);

int main(int argc, char **argv)
{
    char *p;
    FILE *fp;
    FILE *fq;
    int rc;
    int x;
    int rlad = 0; /* relocation address */
    char *buf;
    int len;
    int entry;

    if (argc <= 2)
    {
        printf("usage: loadzero <input file> <output file>\n");
        printf("plus optional relocation address, 0x for hex\n");
        printf("designed to operate over an MVS PE executable\n");
        return (EXIT_FAILURE);
    }
    fq = fopen(*(argv + 2), "wb");
    if (fq == NULL)
    {
        printf("failed to open file %s\n", *(argv + 2));
        return (EXIT_FAILURE);
    }
    fp = fopen(*(argv + 1), "rb");
    if (fp == NULL)
    {
        printf("failed to open input file\n");
        return (EXIT_FAILURE);
    }
    
    if (argc > 3)
    {
        rlad = strtoul(*(argv + 3), NULL, 0);
    }
    
    buf = malloc(MAXEXE);
    if (buf == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    
    len = fread(buf, 1, MAXEXE, fp);
    if (len == MAXEXE)
    {
        printf("executable is too big\n");
        return (EXIT_FAILURE);
    }
    
    if (ferror(fp))
    {
        printf("file read error\n");
        return (EXIT_FAILURE);
    }
    
    if (fixPE(buf, &len, &entry, rlad) != 0)
    {
        printf("error fixing PE\n");
        return (EXIT_FAILURE);
    }
    printf("entry point is %x\n", entry);
    
    fwrite(buf, 1, len, fq);
    if (ferror(fq))
    {
        printf("problem writing output file\n");
        return (EXIT_FAILURE);
    }
    fclose(fq);
    return (0);
}


/* Structures here are documented in Appendix B and E of
   MVS Program Management: Advanced Facilities SA22-7644-01
   and Appendix B of OS/390 DFSMSdfp Utilities SC26-7343-00
*/

#define PE_DEBUG 0

static int fixPE(char *buf, int *len, int *entry, int rlad)
{
    char *p;
    char *q;
    int z;
    typedef struct {
        char pds2name[8];
        char unused1[19];
        char pds2epa[3];
        char pds2ftb1;
        char pds2ftb2;
        char pds2ftb3;
    } IHAPDS;
    IHAPDS *ihapds;
    int rmode;
    int amode;
    int ent;
    int rec = 0;
    int corrupt = 1;
    int rem = *len;
    int l;
    int l2;
    int lastt = -1;
    char *lasttxt = NULL;
    char *upto = buf;
    int initlen = *len;
    
    if ((*len <= 8) || (*((int *)buf + 1) != 0xca6d0f))
    {
        printf("Not an MVS PE executable\n");
        return (-1);
    }
#if PE_DEBUG
    printf("MVS PE total length is %d\n", *len);
#endif
    p = buf;
    while (1)
    {
        rec++;
        l = *(short *)p;
        /* keep track of remaining bytes, and ensure they really exist */
        if (l > rem)
        {
            break;
        }
        rem -= l;
#if PE_DEBUG
        printf("rec %d, offset %d is len %d\n", rec, p - buf, l);
#endif
#if 0
        if (1)
        {
            for (z = 0; z < l; z++)
            {
                printf("z %d %x %c\n", z, p[z], isprint(p[z]) ? p[z] : ' ');
            }
        }
#endif
        if (rec == 3) /* directory record */
        {
            /* there should only be one directory entry, 
               which is 4 + 276 + 12 */
            if (l < 292)
            {
                break;
            }
            q = p + 24;
            l2 = *(short *)q;
            if (l2 < 32) break;
            ihapds = (IHAPDS *)(q + 2);
            rmode = ihapds->pds2ftb2 & 0x10;
            amode = (ihapds->pds2ftb2 & 0x0c) >> 2;
            ent = 0;
            memcpy((char *)&ent + sizeof(ent) - 3, ihapds->pds2epa, 3);
            *entry = (int)(buf + ent);
#if PE_DEBUG
            printf("module name is %.8s\n", ihapds->pds2name);
            printf("rmode is %s\n", rmode ? "ANY" : "24");
            printf("amode is ");
            if (amode == 0)
            {
                printf("24");
            }
            else if (amode == 2)
            {
                printf("31");
            }
            else if (amode == 1)
            {
                printf("64");
            }
            else if (amode == 3)
            {
                printf("ANY");
            }
            printf("\n");
            printf("entry point is %x\n", ent);
#endif            
        }
        else if (rec > 3)
        {
            int t;
            int r2;
            int l2;
            int term = 0;
            
            if (l < (4 + 12))
            {
                break;
            }
            q = p + 4 + 10;
            r2 = l - 4 - 10;
            while (1)
            {
                l2 = *(short *)q;
                r2 -= sizeof(short);
                if (l2 > r2)
                {
                    term = 1;
                    break;
                }
                r2 -= l2;

                if (l2 == 0) break;
                q += sizeof(short);
#if PE_DEBUG
                printf("load module record is of type %2x (len %5d)"
                       " offset %d\n", 
                       *q, l2, q - p);
#endif

                t = *q;
                if ((lastt == 1) || (lastt == 3) || (lastt == 0x0d))
                {
#if PE_DEBUG
                    printf("rectype: program text\n");
#endif
                    lasttxt = q;
                    memmove(upto, q, l2);
                    upto += l2;
                    t = -1;
                    if (lastt == 0x0d)
                    {
                        term = 1;
                        corrupt = 0;
                        break;
                    }
                }
                else if (t == 0x20)
                {
                    /* printf("rectype: CESD\n"); */
                }
                else if (t == 1)
                {
                    /* printf("rectype: Control\n"); */
                }
                else if (t == 0x0d)
                {
                    /* printf("rectype: Control, about to end\n"); */
                }
                else if (t == 2)
                {
                    /* printf("rectype: RLD\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
                }
                else if (t == 3)
                {
                    int l3;
                    
                    /* printf("rectype: Dicionary = Control + RLD\n"); */
                    l3 = *(short *)(q + 6) + 16;
#if 0
                    printf("l3 is %d\n", l3);
#endif
                    if (processRLD(buf, rlad, q, l3) != 0)
                    {
                        term = 1;
                        break;
                    }
                }
                else if (t == 0x0e)
                {
                    /* printf("rectype: Last record of module\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
                    term = 1;
                    corrupt = 0;
                    break;
                }
                else if (t == 0x80)
                {
                    /* printf("rectype: CSECT\n"); */
                }
                else
                {
                    /* printf("rectype: unknown %x\n", t); */
                }
#if 0
                if ((t == 0x20) || (t == 2))
                {
                    for (z = 0; z < l; z++)
                    {
                        printf("z %d %x %c\n", z, q[z], 
                               isprint(q[z]) ? q[z] : ' ');
                    }
                }
#endif
                lastt = t;

                q += l2;
                if (r2 == 0)
                {
#if PE_DEBUG
                    printf("another clean exit\n");
#endif
                    break;
                }
                else if (r2 < (10 + sizeof(short)))
                {
                    /* printf("another unclean exit\n"); */
                    term = 1;
                    break;
                }
                r2 -= 10;
                q += 10;
            }
            if (term) break;            
        }
        p = p + l;
        if (rem == 0)
        {
#if PE_DEBUG
            printf("breaking cleanly\n");
#endif
        }
        else if (rem < 2)
        {
            break;
        }
    }
    if (corrupt)
    {
        printf("corrupt module\n");
        return (-1);
    }
#if 0
    printf("dumping new module\n");
#endif
    *len = upto - buf; /* return new module length */
    return (0);
}


static int processRLD(char *buf, int rlad, char *rld, int len)
{
    int l;
    char *r;
    int cont = 0;
    char *fin;
    int negative;
    int ll;
    int a;
    int newval;
    int *zaploc;
    
    r = rld + 16;
    fin = rld + len;
    while (r != fin)
    {
        if (!cont)
        {
            r += 4; /* skip R & P */
            if (r >= fin)
            {
                printf("corrupt1 at position %x\n", r - rld - 4);
                return (-1);
            }
        }
        negative = *r & 0x02;
        if (negative)
        {
            printf("got a negative adjustment - unsupported\n");
            return (-1);
        }
        ll = (*r & 0x0c) >> 2;
        ll++;
        if ((ll != 4) && (ll != 3))
        {
            printf("untested and unsupported relocation %d\n", ll);
            return (-1);
        }
        if (ll == 3)
        {
            if (rlad > 0xffffff)
            {
                printf("AL3 prevents relocating this module to %x\n", rlad);
                return (-1);
            }
        }
        cont = *r & 0x01; /* do we have A & F continous? */
        r++;
        if ((r + 3) > fin)
        {
            printf("corrupt2 at position %x\n", r - rld);
            return (-1);
        }
        a = 0;
        memcpy((char *)&a + sizeof(a) - 3, r, 3);
        /* +++ need bounds checking on this OS code */
        /* printf("need to zap %d bytes at offset %6x\n", ll, a); */
        zaploc = (int *)(buf + a - ((ll == 3) ? 1 : 0));
        newval = *zaploc;
        /* printf("which means that %8x ", newval); */
        newval += rlad;
        /* printf("becomes %8x\n", newval); */
        *zaploc = newval;
        r += 3;
    }
    return (0);
}
XX
//*
//LKED.SYSLMOD DD DSN=&&TEMPL(LOADZERO),DISP=(OLD,PASS)
//*
//IEBCOPY  EXEC PGM=IEBCOPY
//SYSUT1   DD DSN=UDOS.LINKLIB,DISP=SHR
//SYSUT2   DD DSN=&&COPY,SPACE=(CYL,(10,10)),UNIT=SYSALLDA,
//         DISP=(NEW,PASS)
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 COPY INDD=((SYSUT1,R)),OUTDD=SYSUT2
 SELECT MEMBER=KERNEL
/*
//*
//LOADZERO EXEC PGM=LOADZERO,PARM='dd:in dd:out 36864'
//STEPLIB  DD DSN=&&TEMPL,DISP=(OLD,PASS)
//IN       DD DSN=&&COPY,DISP=(OLD,PASS)
//OUT      DD  DSN=HERC02.ZIP,DISP=(,KEEP),UNIT=TAPE,
//         LABEL=(1,SL),VOL=SER=MFTOPC,
//         DCB=(RECFM=U,LRECL=0,BLKSIZE=8000)
//SYSIN    DD DUMMY
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//*