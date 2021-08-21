#include <printf.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vfs.h>

int g_stdio_fd = 0;

int kgetc(void) {
    char input[2];
read:
    //sclp_read(input, 1);
    if(input[0] == '@') {
        goto read;
    }
    return (int)input[0];
}

int kputc(int c) {
    char ch = (char)c;
    int fd;
    vfs_write(g_stdio_fd, &ch, sizeof(ch));
    return 0;
}

static char numbuf[32];

static char tmpbuf[8192];
char *out_ptr = (char *)&tmpbuf;

void kflush(void) {
    size_t i;
    *out_ptr = '\0';
    out_ptr = (char *)&tmpbuf;

    for(i = 0; i < strlen(out_ptr); i++) {
        kputc(out_ptr[i]);
    }
    return;
}

static void print_number(unsigned long val, int base) {
    char *buf_ptr = (char *)&numbuf;
    if(!val) {
        *(buf_ptr++) = '0';
    } else {
        while(val) {
            char rem = (char)(val % (unsigned long)base);
            *(buf_ptr++) = (rem >= 10) ? rem - 10 + 'A' : rem + '0';
            val /= (unsigned long)base;
        }
    }
    while(buf_ptr != (char *)&numbuf) {
        *(out_ptr++) = *(--buf_ptr);
    }
    return;
}

static void print_inumber(signed long val, int base) {
    char *buf_ptr = (char *)&numbuf;
    if(!val) {
        *(buf_ptr++) = '0';
    } else {
        while(val) {
            char rem = (char)(val % (long)base);
            *(buf_ptr++) = (rem >= 10) ? rem - 10 + 'A' : rem + '0';
            val /= base;
        }
    }

    if(val < 0) {
        *(out_ptr++) = '-';
    }
    while(buf_ptr != (char *)&numbuf) {
        *(out_ptr++) = *(--buf_ptr);
    }
    return;
}

int kvprintf(const char *fmt, va_list args) {
    size_t i;

    while(*fmt != '\0') {
        if(*fmt == '\n') {
            *(out_ptr++) = *(fmt++);
            kflush();
            continue;
        }

        if(*fmt == '%') {
            ++fmt;
            if(!strncmp(fmt, "s", 1)) {
                const char *str = va_arg(args, const char *);
                if(str == NULL) {
                    kprintf("(nil)");
                } else {
                    for(i = 0; i < strlen(str); i++) {
                        *(out_ptr++) = str[i];
                    }
                }
            } else if(!strncmp(fmt, "zu", 2)) {
                size_t val = va_arg(args, size_t);
                print_number((unsigned long)val, 10);
                ++fmt;
            } else if(!strncmp(fmt, "u", 1)) {
                unsigned int val = va_arg(args, unsigned int);
                print_number((unsigned long)val, 10);
            } else if(!strncmp(fmt, "p", 1)) {
                unsigned val = va_arg(args, unsigned);
                print_number((unsigned long)val, 16);
            } else if(!strncmp(fmt, "x", 1)) {
                unsigned val = va_arg(args, unsigned);
                print_number((unsigned long)val, 16);
            } else if(!strncmp(fmt, "i", 1)) {
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