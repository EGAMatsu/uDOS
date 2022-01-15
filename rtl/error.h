#ifndef ERROR_H
#define ERROR_H

#define RtlPrintError _Zrerrp
int RtlPrintError(const char *str);
#define RtlPrintWarn _Zrwrnp
int RtlPrintWarn(const char *str);
#define RtlPrintLog _Zrlogp
int RtlPrintLog(const char *str);

#endif
