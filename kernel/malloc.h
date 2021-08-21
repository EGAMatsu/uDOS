#ifndef _MALLOC_H
#define _MALLOC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <pmm.h>

void *kmalloc(size_t size);
void *kmalloc_array(size_t n, size_t size);
void *kzalloc(size_t size);
void *kzalloc_array(size_t n, size_t size);
void *krealloc(void *ptr, size_t size);
void *krealloc_array(void *ptr, size_t n, size_t size);
void kfree(void *ptr);

#ifdef __cplusplus
}
#endif
#endif