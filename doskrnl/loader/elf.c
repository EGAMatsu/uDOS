#include <loader/elf.h>
#include <memory.h>

struct Elf32Shdr *ExGetElf32Shdr(
    struct Elf32Header *hdr,
    int idx)
{
    return (struct Elf32Shdr *)((uintptr_t)hdr
        + (uintptr_t)hdr->sect_tab + (hdr->sect_tab_entry_size * idx));
}

struct Elf32Shdr *ExGetElf32StringShdr(
    struct Elf32Header *hdr)
{
    return ExGetElf32Shdr(hdr, (int)hdr->str_shtab_idx);
}

const char *ExGetElf32String(
    struct Elf32Header *hdr,
    int offset)
{
    struct Elf32Shdr *strtab = ExGetElf32StringShdr(hdr);
    if(strtab == NULL) {
        return NULL;
    }

    return (const char *)((uintptr_t)hdr + (uintptr_t)strtab->offset
        + (uintptr_t)offset);
}

int ExDoElf32Relocation(
    struct Elf32Header *hdr,
    struct Elf32RelEntry* rel,
    struct Elf32Shdr *reltab)
{
    struct Elf32Shdr *target = ExGetElf32Shdr(hdr, reltab->info);
    uintptr_t addr = (uintptr_t)hdr + target->offset;
    uint32_t *ref = (uint32_t *)(addr + rel->offset);

    /* TODO: Do relocations */
    return 0;
}

int ExCheckElf32IsValid(
    const struct Elf32Header *hdr)
{
    /* Check signature of ELF file */
    if(KeCompareMemory(&hdr->id, ELF_MAGIC, 4) != 0) {
        return -1;
    }
    return 0;
}

int ExLoadElf32Section(
    const struct Elf32Header *hdr,
    struct Elf32Shdr *sect)
{
    /* Section not present on file */
    if(sect->type == SHT_NOBITS) {
        /* Skip empty sections */
        if(sect->size == 0) {
            return -1;
        }

        if(sect->flags & SHF_ALLOC) {
            void *p;
            p =  MmAllocatePhysical(sect->size, sect->addralign);

            sect->offset = (unsigned int)p - (unsigned int)hdr;
        }
    }
    return 0;
}

#include <debug/printf.h>
#include <memory.h>
int ExLoadElfFromBuffer(
    void *buffer,
    size_t n)
{
    void *end_buffer = (void *)((uintptr_t)buffer + n);
    const struct Elf32Header *hdr = (const struct Elf32Header *)buffer;
    size_t i;

    /* Check validity of the ELF */
    if(ExCheckElf32IsValid(hdr) != 0) {
        KePanic("Invalid ELF32 file\r\n");
    }

    /* Iterate over the sections and load them on the storage */
    for(i = 0; i < hdr->n_sect_tab_entry; i++) {
        struct Elf32Shdr *shdr = ExGetElf32Shdr(hdr, i);
#if defined(DEBUG)
        kprintf("Section: %s\r\n", (const char *)ExGetElf32String(shdr, shdr->name));
#endif

        /* Sections not present on the file */
        if(shdr->type == SHT_NOBITS) {
            if(shdr->size == 0) {
                continue;
            }

            /* Sections that needs to be allocated */
            if(shdr->flags & SHF_ALLOC) {
                shdr->addr = MmAllocateZero(shdr->size);
                kprintf("Allocated %p(%zu)\r\n", (uintptr_t)shdr->addr, (size_t)shdr->size);
            }
        } else {
            /* TODO: Map pages to the address in the virtual space */
            void *addr = (void *)shdr->addr;
            if(addr == NULL) {
                continue;
            }

            KeCopyMemory(addr, (uintptr_t)hdr + shdr->offset, shdr->size);
            kprintf("ProgBit %p(%zu)\r\n", (uintptr_t)shdr->addr, (size_t)shdr->size);
        }
    }

    typedef void (*entry_handler_t)(void);
    entry_handler_t hdl = (entry_handler_t)hdr->entry;
    hdl();
    return 0;
}