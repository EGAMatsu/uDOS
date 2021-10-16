#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H

#if defined(DEBUG)
#   include <panic.h>
#   define DEBUG_ASSERT(expr)\
    if(!(expr)) {\
        kpanic("Assertion " #expr " failed!\r\n");\
    }
#else
#   define DEBUG_ASSERT(expr) 0;
#endif

#endif