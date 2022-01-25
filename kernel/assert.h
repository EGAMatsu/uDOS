#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H

#if defined(DEBUG)
#   include <panic.h>
#   define DEBUG_ASSERT(expr, ...) \
    if(!(expr)) { \
        KePanic("Assertion " #expr " failed!\r\n");\
    }
#   define DEBUG_ASSERT_MSG(expr, msg, ...) \
    if(!(expr)) { \
        KePanic("Assertion " #expr " failed!: " msg "\r\n");\
    }
#else
#   define DEBUG_ASSERT(expr, ...)
#   define DEBUG_ASSERT_MSG(expr, msg, ...)
#endif

#endif
