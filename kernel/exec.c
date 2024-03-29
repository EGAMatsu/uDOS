#include <exec.h>
#include <memory.h>
#include <mm.h>
#include <panic.h>
#include <printf.h>
#include <memory.h>

struct elf32_shdr *ExGetElf32Shdr(struct elf32_header *hdr, int idx)
{
    return (struct elf32_shdr *)((unsigned int)hdr + (unsigned int)hdr->sect_tab + (hdr->sect_tab_entry_size * idx));
}

struct elf32_shdr *ExGetElf32StringShdr(struct elf32_header *hdr)
{
    return ExGetElf32Shdr(hdr, (int)hdr->str_shtab_idx);
}

const char *ExGetElf32String(struct elf32_header *hdr, int offset)
{
    struct elf32_shdr *strtab = ExGetElf32StringShdr(hdr);
    if(strtab == NULL) {
        return NULL;
    }

    return (const char *)((unsigned int)hdr + (unsigned int)strtab->offset
        + (unsigned int)offset);
}

int ExDoElf32Relocation(struct elf32_header *hdr, struct elf32_rel* rel, struct elf32_shdr *reltab)
{
    struct elf32_shdr *target = ExGetElf32Shdr(hdr, reltab->info);
    unsigned int addr = (unsigned int)hdr + target->offset;
    uint32_t *ref = (uint32_t *)(addr + rel->offset);

    /* TODO: Do relocations */
    return 0;
}

int ExCheckElf32IsValid(const struct elf32_header *hdr)
{
    /* Check signature of ELF file */
    if(KeCompareMemory(&hdr->id, ELF_MAGIC, 4) != 0) {
        return -1;
    }
    return 0;
}

int ExLoadElf32Section(const struct elf32_header *hdr, struct elf32_shdr *sect)
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

int ExLoadElfFromBuffer(void *buffer, size_t n)
{
    void *end_buffer = (void *)((unsigned int)buffer + n);
    const struct elf32_header *hdr = (const struct elf32_header *)buffer;
    size_t i;

    /* Check validity of the ELF */
    if(ExCheckElf32IsValid(hdr) != 0) {
        KePanic("Invalid ELF32 file\r\n");
    }

    /* Iterate over the sections and load them on the storage */
    for(i = 0; i < hdr->n_sect_tab_entry; i++) {
        struct elf32_shdr *shdr = ExGetElf32Shdr(hdr, i);
#if defined(DEBUG)
        KeDebugPrint("Section: %s\r\n", (const char *)ExGetElf32String(shdr, shdr->name));
#endif

        /* Sections not present on the file */
        if(shdr->type == SHT_NOBITS) {
            if(shdr->size == 0) {
                continue;
            }

            /* Sections that needs to be allocated */
            if(shdr->flags & SHF_ALLOC) {
                shdr->addr = MmAllocateZero(shdr->size);
                KeDebugPrint("Allocated %p(%zu)\r\n", (unsigned int)shdr->addr, (size_t)shdr->size);
            }
        } else {
            /* TODO: Map pages to the address in the virtual space */
            void *addr = (void *)shdr->addr;
            if(addr == NULL) {
                continue;
            }

            KeCopyMemory(addr, (unsigned int)hdr + shdr->offset, shdr->size);
            KeDebugPrint("ProgBit %p(%zu)\r\n", (unsigned int)shdr->addr, (size_t)shdr->size);
        }
    }

    typedef void (*entry_handler_t)(void);
    entry_handler_t hdl = (entry_handler_t)hdr->entry;
    hdl();
    return 0;
}
