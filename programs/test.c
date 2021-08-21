#include <stdint.h>
#include <stddef.h>

double bss_sym[255] = { 0.f };
size_t bss_sym_len = 255;
size_t i;

int add(int x, int y) {
    return (x + y);
}

int crash_causer(void) {
    unsigned char *p = NULL;
    *p = 0;
    return 0;
}

int main(int argc, char **argv) {
    bss_sym[0] = add(1, 1);
    i = bss_sym[0];
    for(i = 0; i < bss_sym_len; i++) {
        bss_sym[i] = bss_sym_len * i + bss_sym[i] * i / (bss_sym[i] - i);
    }
    crash_causer();
    return 0;
}