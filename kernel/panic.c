#include <panic.h>
#include <printf.h>
#include <memory.h>

void KePanic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    g_stdout_fd = NULL;
    g_stdin_fd = NULL;
    
    kvprintf(fmt, args);
    kflush();
    
    va_end(args);
    
    KePrint("Taking down CPU...\r\n");
    HwSignalCPU(HwCPUID(), S390_SIGP_STOP);
    
    KePrint("CPU should be down by now...\r\n");
    while(1) {
        
    }
}
