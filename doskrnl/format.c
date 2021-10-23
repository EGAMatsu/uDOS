#include <stdint.h>
#include <stddef.h>

void format_int(char *buffer, int num) {
    char numbuf[16] = {0};
    size_t i, j = 0;
    
    if(!num) {
        KeCopyMemory(&buffer[0], "0", 2);
        return;
    }

    if(num < 0) {
        numbuf[j++] = '-';
        num = -num;
    }

    while(num) {
        numbuf[j++] = num % 10 + '0';
        num /= 10;
    }

    for(i = 0; i < j; i++) {
        buffer[i] = numbuf[j - i - 1];
    }
    buffer[i] = '\0';
    return;
}

void format_address(char *buffer, void *p) {
    uintptr_t num = (uintptr_t)p;
    char numbuf[16] = {0};
    size_t i, j = 0;

    if(!num) {
        KeCopyMemory(&buffer[0], "X'00", 5);
        return;
    }

    KeCopyMemory(&numbuf[0], "X'", 2);
    j += 2;

    while(num) {
        uintptr_t rem = (uintptr_t)p % 16;
        numbuf[j++] = (rem >= 10) ? rem - 10 + 'A' : rem + '0';
        num /= 16;
    }

    for(i = 0; i < j; i++) {
        buffer[i] = numbuf[j - i - 1];
    }
    buffer[i] = '\0';
    return;
}