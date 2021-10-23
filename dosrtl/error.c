#include <error.h>

int RtlPrintError(
    const char *str)
{
    RtlDebugPrint(str);
    return 0;
}

int RtlPrintWarn(
    const char *str)
{
    RtlPrintWarn("RtlPrintLog not implemented\r\n");
    return 0;
}

int RtlPrintLog(
    const char *str)
{
    RtlPrintError("RtlPrintLog not implemented\r\n");
    return 0;
}