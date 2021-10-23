#ifndef DEBUG_H
#define DEBUG_H

void RtlDebugPrint(const char *str);

#if defined(DEBUG)
#define DEBUG_ASSERT(expr)\
    if(!(expr)) {\
        RtlDebugPrint("Assertion " #expr "failed!\r\n")\
    }
#else
#define DEBUG_ASSERT(expr)
#endif

#endif