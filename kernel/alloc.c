#include <alloc.h>
#include <string.h>

void *kmalloc(
    size_t size)
{
    void *ptr;
    ptr = pmm_alloc(size, 16);
    return ptr;
}

void *kmalloc_array(
    size_t n,
    size_t size)
{
    return kmalloc(n * size);
}

void *kzalloc(
    size_t size)
{
    void *ptr;
    ptr = kmalloc(size);
    if(ptr == NULL) {
        kpanic("Out of memory");
    }
    memset(ptr, 0, size);
    return ptr;
}

void *kzalloc_array(
    size_t n,
    size_t size)
{
    return kzalloc(n * size);
}

void *krealloc(
    void *ptr,
    size_t size)
{
    void *old_ptr = ptr;
    if(old_ptr == NULL) {
        return kmalloc(size);
    }
    
    ptr = kmalloc(size);
    if(ptr == NULL) {
        return NULL;
    }
    memmove(ptr, old_ptr, size);
    kfree(old_ptr);
    return ptr;
}

void *krealloc_array(
    void *ptr,
    size_t n,
    size_t size)
{
    return krealloc(ptr, n * size);
}

void kfree(
    void *ptr)
{
    if(ptr == NULL) {
        return;
    }
    
    pmm_free(ptr);
    return;
}