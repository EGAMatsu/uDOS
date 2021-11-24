#ifndef S390_INTERRUPT_H
#define S390_INTERRUPT_H

#include <stdint.h>

/* Assembly stubs */
#define KeAsmSupervisorCallHandler _Asvchdl
extern void KeAsmSupervisorCallHandler(void);
#define KeAsmProgramCheckHandler _Apchdl
extern void KeAsmProgramCheckHandler(void);
#define KeAsmExternalHandler _Aexthdl
extern void KeAsmExternalHandler(void);
#define KeAsmMachineCheckHandler _Amchdl
extern void KeAsmMachineCheckHandler(void);
#define KeAsmIOHandler _Aiohdl
extern void KeAsmIOHandler(void);

/* C counterparts */
#define KeSupervisorCallHandler _Zhsvc
void KeSupervisorCallHandler(void);
#define KeProgramCheckHandler _Zhpc
void KeProgramCheckHandler(void);
#define KeExternalHandler _Zhext
void KeExternalHandler(void);
#define KeMachineCheckHandler _Zhmc
void KeMachineCheckHandler(void);
#define KeIOHandler _Zhio
void KeIOHandler(void);

#endif
