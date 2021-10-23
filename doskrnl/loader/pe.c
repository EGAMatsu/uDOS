#include <loader/pe.h>
#include <debug/panic.h>
#include <mm/mm.h>

struct PeReader *ExCreatePeReader(
    void)
{
    struct PeReader *reader;

    reader = MmAllocateZero(sizeof(struct PeReader));
    if(reader == NULL) {
        KePanic("Out of memory");
    }
    return reader;
}

int ExReadPeFromBuffer(
    struct PeReader *reader,
    void *buffer,
    size_t n)
{
    void *end_buffer = (void *)((uintptr_t)buffer + n);
    struct PeCesdRecord cesd;
    struct PeSymRecord sym;
    struct PeTranslationRecord trans;

    struct PeEsdRecord *esd;

    cesd.data = MmAllocateZero(248);
    if(cesd.data == NULL) {
        KePanic("Out of memory");
    }

    sym.data = MmAllocateZero(248);
    if(sym.data == NULL) {
        KePanic("Out of memory");
    }

    trans.data = MmAllocateZero(1024);
    if(trans.data == NULL) {
        KePanic("Out of memory");
    }

    reader->buffer = buffer;
    while((ptrdiff_t)buffer < (ptrdiff_t)end_buffer) {
        unsigned char id = *reader->buffer;
        reader->buffer++;

        switch(id) {
        case PE_CESD_ID:
            KeCopyMemory(&cesd.flag, reader->buffer, 1);
            reader->buffer += 1;

            KeCopyMemory(&cesd.spare, reader->buffer, 2);
            reader->buffer += 2;

            /* Check that the binary zeroes are actually zeroes */
            if(cesd.spare != 0) {
                kprintf("CESD spare area is not zero\r\n");
                return -1;
            }

            KeCopyMemory(&cesd.essid, reader->buffer, 2);
            reader->buffer += 2;

            KeCopyMemory(&cesd.length, reader->buffer, 2);
            reader->buffer += 2;
            if(cesd.length < 8) {
                kprintf("CESD length is smaller than 4\r\n");
                return -1;
            }

            KeCopyMemory(cesd.data, reader->buffer, cesd.length);
            reader->buffer += cesd.length;

            kprintf("CESD Id=%u, Len=%u, Spare=%u, Flag=%u\n",
                (unsigned)cesd.flag, (unsigned)cesd.spare,
                (unsigned)cesd.essid, (unsigned)cesd.length);
            break;
        case PE_SYM_ID:
            KeCopyMemory(&sym.subtype, reader->buffer, 1);
            reader->buffer += 1;

            KeCopyMemory(&sym.length, reader->buffer, 2);
            reader->buffer += 2;

            KeCopyMemory(sym.data, reader->buffer, sym.length);
            reader->buffer += sym.length;

            /* Parse the data */
            switch(sym.subtype) {
            case PE_SYM_TYPE_ESD:
                esd = sym.data;
                while((ptrdiff_t)esd < (ptrdiff_t)((uintptr_t)sym.data + sym.length)) {
                    kprintf("ESD Name: %s\r\n", esd->name);

                    /* Go to the next ESD record */
                    esd++;
                }
                break;
            case PE_SYM_TYPE_NOT_ESD:
                break;
            default:
                break;
            }
            break;
        case PE_TRANS_ID:
            KeCopyMemory(&trans.zero, reader->buffer, 1);
            reader->buffer += 1;
            if(trans.zero != 0) {
                kprintf("Translation record zero is not zero\r\n");
                return -1;
            }

            KeCopyMemory(&trans.length, reader->buffer, 2);
            reader->buffer += 2;

            KeCopyMemory(trans.data, reader->buffer, trans.length);
            reader->buffer += trans.length;

            kprintf("Translation Zero=%u,Len=%u\r\n", (unsigned)trans.zero,
                (unsigned)trans.length);
            break;
        default:
            break;
        }

        reader->buffer++;
    }
    return 0;
}
