/*
 * z/OS Dataset filesystem support
 */

#ifndef ZDSFS_H
#define ZDSFS_H

#include <stdint.h>
#include <fs/fs.h>

struct zdsfs_dscb_fmt1 {
    char ds_name[44];
    char format_id;
    uint8_t unused[60];
    uint8_t ext_type;
    uint8_t ext_seq_num;
    uint16_t start_cc;
    uint16_t start_hh;
    uint16_t end_cc;
    uint16_t end_hh;
} __attribute__((packed));

int ModGetZdsfsFile(struct fs_handle *hdl, struct fs_fdscb *out_fdscb,
    const char *name);

#endif
