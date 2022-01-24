#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H

#if defined(DEBUG)
#   include <panic.h>
#   define DEBUG_ASSERT(expr) \
    if(!(expr)) { \
        KePanic(__FILE__ ":" __LINE__ ": Assertion " #expr " failed!\r\n");\
    }
#   define DEBUG_ASSERT_MSG(expr, msg) \
    if(!(expr)) { \
        KePanic(__FILE__ ":" __LINE__ ": Assertion " #expr " failed!: " msg "\r\n");\
    }
#else
#   define DEBUG_ASSERT(expr) 0;
#   define DEBUG_ASSERT_MSG(expr) 0;
#endif

#endif
