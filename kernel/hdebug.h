#ifndef HDEBUG_H
#define HDEBUG_H

#include <stddef.h>
#include <Fs/Fs.h>

void __DIAG8(void *text, unsigned int len);

#define ModWriteHercDebug _Zmhwdb
int ModWriteHercDebug(struct fs_handle *hdl, const void *buf, size_t n);
#define ModInitHercDebug _Zmihdb
int ModInitHercDebug(void);

#endif
