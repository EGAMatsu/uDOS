#include <elf.h>
#include <string.h>

struct elf32_shdr *elf32_get_shdr(
    struct elf32_header *hdr,
    int idx)
{
    return (struct elf32_shdr *)((uintptr_t)hdr
        + (uintptr_t)hdr->sect_tab + (hdr->sect_tab_entry_size * idx));
}

struct elf32_shdr *elf32_get_string_shdr(
    struct elf32_header *hdr)
{
    return elf32_get_shdr(hdr, (int)hdr->str_shtab_idx);
}

const char *elf32_get_string(
    struct elf32_header *hdr,
    int offset)
{
    struct elf32_shdr *strtab = elf32_get_string_shdr(hdr);
    if(strtab == NULL) {
        return NULL;
    }

    return (const char *)((uintptr_t)hdr + (uintptr_t)strtab->offset
        + (uintptr_t)offset);
}

int elf32_do_reloc(
    struct elf32_header *hdr,
    struct elf32_rel* rel,
    struct elf32_shdr *reltab)
{
    struct elf32_shdr *target = elf32_get_shdr(hdr, shdr->info);
    uintptr_t addr = (uintptr_t)hdr + target->offset;
    uint32_t *ref = (uint32_t *)(addr + rel->offset);

    /* TODO: Do relocations */
    return 0;
}

int elf32_is_valid(
    const struct elf32_header *hdr)
{
    /* Check signature of ELF file */
    if(memcmp(hdr->magic, ELF_MAGIC, 4) != 0) {
        return -1;
    }

    return 0;
}