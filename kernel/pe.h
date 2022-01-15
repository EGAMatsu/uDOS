#ifndef PE_H
#define PE_H

#include <stdint.h>
#include <stddef.h>

#define PE_SYM_TYPE_ESD 0x80
#define PE_SYM_TYPE_NOT_ESD 0x00

#define PE_SYM_ID 0x40
struct PeSymRecord {
    uint8_t subtype;
    uint16_t length;
    void *data;
};

#define PE_CESD_ID 0x20
struct PeCesdRecord {
    uint8_t flag;
    uint16_t spare;
    uint16_t essid;
    uint16_t length;
    void *data;
};

struct PeEsdRecord {
    uint8_t name[8];
    uint8_t type;
    uint8_t address[3];
    uint8_t flags;
    uint8_t id[3];
};

#define PE_TRANS_ID 0x10
struct PeTranslationRecord {
    uint8_t zero;
    uint16_t length;
    void *data;
};

struct PeReader {
    struct PeSymRecord *sym_records;
    size_t n_sym_records;

    struct PeCesdRecord *cesd_records;
    size_t n_cesd_records;

    struct PeEsdRecord *esd_records;
    size_t n_esd_records;

    struct PeTranslationRecord *trans_records;
    size_t n_trans_records;

    uint8_t *buffer;
};

struct PeReader *ExCreatePeReader(void);
int ExReadPeFromBuffer(struct PeReader *reader, void *buffer, size_t n);

#endif
