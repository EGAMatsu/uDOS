#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <alloc.h>
#include <memory.h>
#include <debug.h>

/* Creates an instance of a list without declaring a specific type */
#define LIST_SINGLE_INSTANCE(name, type)\
    struct { type *elems; size_t n_elems; }name;

/* Declares a type of list */
#define LIST_TYPE(name, type)\
    typedef struct { type *elems; size_t n_elems; }name;

/* Pushes new_elem into the front of a list */
#define LIST_PUSH(list, new_elem, fail_cb)\
    DEBUG_ASSERT(sizeof(*new_elem) == sizeof((list).elems[0]));\
    (list).elems = RtlReallocateMemory((list).elems, (list).n_elems + 1);\
    if((list).elems == NULL) { fail_cb; }\
    RtlCopyMemory(\
        &((list).elems[(list).n_elems]),\
        (new_elem),\
        sizeof((list).elems[0])\
    );\
    (list).n_elems++;\

/* Pops old_elem from the back of a list */
#define LIST_POP(list, old_elem, fail_cb)\
    RtlCopyMemory(\
        (old_elem),\
        &((list).elems[(list).n_elems]),\
        sizeof((list).elems[0])\
    );\
    (list).elems = RtlReallocateMemory((list).elems, (list).n_elems - 1);\
    if((list).elems == NULL) { fail_cb; }\
    (list).n_elems--;

#define LIST_SIZE(list) (list).n_elems

#endif