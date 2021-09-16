#include <printf.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vfs.h>

int g_stdio_fd = -1;

int diag8_write(int fd, const void *buf, size_t size);

int kgetc(void) { return (int)'A'; }

int kputc(int c) {
    char ch = (char)c;

    if(g_stdio_fd == -1) {
        diag8_write(0, &ch, sizeof(ch));
    } else {
        vfs_write(g_stdio_fd, &ch, sizeof(ch));
    }
    return 0;
}

static char tmpbuf[80];
char *out_ptr = &tmpbuf[0];

void kflush(void) {
    *out_ptr = '\0';
    out_ptr  = &tmpbuf[0];

    if(g_stdio_fd == -1) {
        diag8_write(0, out_ptr, strlen(out_ptr));
    } else {
        vfs_write(g_stdio_fd, out_ptr, strlen(out_ptr));
    }
    return;
}

static void print_number(unsigned long val, int base) {
    static char numbuf[16];
    char *buf_ptr = (char *)&numbuf[0];

    if (!val) {
        *(buf_ptr++) = '0';
    } else {
        while (val) {
            char rem     = (char)(val % (unsigned long)base);
            *(buf_ptr++) = (rem >= 10) ? rem - 10 + 'A' : rem + '0';
            val /= (unsigned long)base;
        }
    }

    while ((ptrdiff_t)buf_ptr != (ptrdiff_t)&numbuf) {
        *(out_ptr++) = *(--buf_ptr);
    }
    return;
}

static void print_inumber(signed long val, int base) {
    static char numbuf[16];
    char *buf_ptr = (char *)&numbuf[0];

    if (!val) {
        *(buf_ptr++) = '0';
    } else {
        while (val) {
            char rem     = (char)(val % (long)base);
            *(buf_ptr++) = (rem >= 10) ? rem - 10 + 'A' : rem + '0';
            val /= base;
        }
    }

    if (val < 0) {
        *(out_ptr++) = '-';
    }
    while ((ptrdiff_t)buf_ptr != (ptrdiff_t)&numbuf) {
        *(out_ptr++) = *(--buf_ptr);
    }
    return;
}

int kvprintf(const char *fmt, va_list args) {
    size_t i;

    while (*fmt != '\0') {
        if (*fmt == '\n' || (uintptr_t)out_ptr - (uintptr_t)&tmpbuf[0] >= 40) {
            *(out_ptr++) = *(fmt++);
            kflush();
            continue;
        }

        if (*fmt == '%') {
            ++fmt;
            if (!strncmp(fmt, "s", 1)) {
                const char *str = va_arg(args, const char *);
                if (str == NULL) {
                    kprintf("(nil)");
                } else {
                    for (i = 0; i < strlen(str); i++) {
                        *(out_ptr++) = str[i];
                    }
                }
            } else if (!strncmp(fmt, "zu", 2)) {
                size_t val = va_arg(args, size_t);
                print_number((unsigned long)val, 10);
                ++fmt;
            } else if (!strncmp(fmt, "u", 1)) {
                unsigned int val = va_arg(args, unsigned int);
                print_number((unsigned long)val, 10);
            } else if (!strncmp(fmt, "p", 1)) {
                unsigned val = va_arg(args, unsigned);
                print_number((unsigned long)val, 16);
            } else if (!strncmp(fmt, "x", 1)) {
                unsigned val = va_arg(args, unsigned);
                print_number((unsigned long)val, 16);
            } else if (!strncmp(fmt, "i", 1)) {
                signed int val = va_arg(args, signed int);
                print_inumber((signed long)val, 10);
            }
            ++fmt;
        } else {
            *(out_ptr++) = *(fmt++);
        }
    }
    return 0;
}

int kprintf(const char *fmt, ...) {
    va_list args;
    int r;

    va_start(args, fmt);
    r = kvprintf(fmt, args);
    va_end(args);
    return r;
}