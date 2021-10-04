/*
 * z/OS Dataset filesystem support
 */

#ifndef ZDSFS_H
#define ZDSFS_H

#include <stdint.h>
#include <vfs.h>

struct zdsfs_dscb_fmt1 {
    char ds1dsnam[44];
    char ds1fmtid;
    uint8_t unused[60];
    uint8_t ext_type;
    uint8_t ext_seq_num;
    uint16_t start_cc;
    uint16_t start_hh;
    uint16_t end_cc;
    uint16_t end_hh;
} __attribute__((packed));

int zdsfs_get_file(struct vfs_node *node, struct vfs_fdscb *out_fdscb,
    const char *name);

#endif