#include <printf.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vfs.h>

int g_stdout_fd = -1;
int g_stdin_fd = -1;

int diag8_write(int fd, const void *buf, size_t size);

int kgetc(void) { return (int)'A'; }

int kputc(int c) {
    char ch = (char)c;

    if(g_stdout_fd == -1) {
        diag8_write(0, &ch, sizeof(ch));
    } else {
        vfs_write(g_stdout_fd, &ch, sizeof(ch));
    }
    return 0;
}

static char tmpbuf[80];
void kflush(void) {
    if(g_stdout_fd == -1) {
        diag8_write(0, &tmpbuf[0], strlen(&tmpbuf[0]));
    } else {
        vfs_write(g_stdout_fd, &tmpbuf[0], strlen(&tmpbuf[0]));
    }
    return;
}

static void itoa(int val, char *str, int base) {
    static char numbuf[16];
    char *buf = (char *)&numbuf[0];
    size_t i = 0, j = 0;

    if (val == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (val) {
        char rem     = (char)(val % (long)base);
        buf[j++] = (rem >= 10) ? rem - 10 + 'A' : rem + '0';
        val /= base;
    }

    if (val < 0) {
        buf[j++] = '-';
    }
    buf[j++] = '\0';

    while (j != 0) {
        str[i++] = buf[j--];
    }
    str[i] = '\0';
    return;
}

int kvsnprintf(char *s, size_t n, const char *fmt, va_list args) {
    size_t i = 0;
    while (*fmt != '\0' && i < n - 1) {
        if (*fmt == '%') {
            ++fmt;
            if (!strncmp(fmt, "s", 1)) {
                const char *str = va_arg(args, const char *);
                if (str == NULL) {
                    kprintf("(nil)");
                } else {
                    strcpy(&s[i], str);
                    i += strlen(str);
                }
            } else if (!strncmp(fmt, "zu", 2)) {
                size_t val = va_arg(args, size_t);
                itoa((int)val, &s[i], 10);
                i = strlen(s);
                ++fmt;
            } else if (!strncmp(fmt, "u", 1)) {
                unsigned int val = va_arg(args, unsigned int);
                itoa((int)val, &s[i], 10);
                i = strlen(s);
            } else if (!strncmp(fmt, "p", 1)) {
                unsigned val = va_arg(args, unsigned);
                itoa((int)val, &s[i], 16);
                i = strlen(s);
            } else if (!strncmp(fmt, "x", 1)) {
                unsigned val = va_arg(args, unsigned);
                itoa((int)val, &s[i], 16);
                i = strlen(s);
            } else if (!strncmp(fmt, "i", 1)) {
                signed int val = va_arg(args, signed int);
                itoa((int)val, &s[i], 10);
                i = strlen(s);
            }
            ++fmt;
        } else {
            s[i++] = *(fmt++);
        }
    }
    s[i] = '\0';
    return 0;
}

int kvprintf(const char *fmt, va_list args) {
    kvsnprintf(&tmpbuf[0], 80, fmt, args);
    kflush();
    return 0;
}

int ksnprintf(char *s, size_t n, const char *fmt, ...) {
    va_list args;
    int r;

    va_start(args, fmt);
    r = kvsnprintf(s, n, fmt, args);
    va_end(args);
    return r;
}

int kprintf(const char *fmt, ...) {
    va_list args;
    int r;

    va_start(args, fmt);
    r = kvprintf(fmt, args);
    va_end(args);
    return r;
}