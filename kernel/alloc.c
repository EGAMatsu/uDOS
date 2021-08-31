#include <string.h>
#include <alloc.h>

void *kmalloc(size_t size) {
    void *ptr;
    ptr = pmm_alloc(size, 8);
    return ptr;
}

void *kmalloc_array(size_t n, size_t size) {
    void *ptr;
    ptr = pmm_alloc(n * size, 8);
    return ptr;
}

void *kzalloc(size_t size) {
    void *ptr;
    ptr = pmm_alloc(size, 8);
    memset(ptr, 0, size);
    return ptr;
}

void *kzalloc_array(size_t n, size_t size) {
    void *ptr;
    ptr = pmm_alloc(n * size, 8);
    memset(ptr, 0, size);
    return ptr;
}

void *krealloc(void *ptr, size_t size) {
    void *old_ptr = ptr;
    if (ptr == NULL) {
        return kmalloc(size);
    }
    ptr = pmm_alloc(size, 8);
    memmove(ptr, old_ptr, size);
    pmm_free(old_ptr);
    return ptr;
}

void *krealloc_array(void *ptr, size_t n, size_t size) {
    void *old_ptr = ptr;
    if (ptr == NULL) {
        return kmalloc(n * size);
    }
    ptr = pmm_alloc(n * size, 8);
    memmove(ptr, old_ptr, n * size);
    pmm_free(old_ptr);
    return ptr;
}

void kfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    pmm_free(ptr);
    return;
}