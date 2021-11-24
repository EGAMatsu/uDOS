#ifndef STDARG_H
#define STDARG_H

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#endif

/* Standard variable argument list */
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

/* Strict ANSI-compliance won't allow va_copy */
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L
#   define va_copy(d,s)	__builtin_va_copy(d, s)
#endif

/* Define it as a compiler extension under the __ namespace */
#define __va_copy(d, s)	__builtin_va_copy(d, s)

/* Builtin type */
#if defined __GNUC__
typedef __gnuc_va_list va_list;
#endif

#endif
