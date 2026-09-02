#include "uaccess.h"
#include "../mm/vmm.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096ULL
#endif

// x86_64 PTE phys field: bits 51:12 (drop NX/flags)
#define PTE_PHYS_MASK 0x000FFFFFFFFFF000ULL

void *user_kptr(process_t *proc, uint64_t uva)
{
    uint64_t page_uva;
    uint64_t phys;

    if (!proc || !uva)
        return 0;

    page_uva = uva & ~(PAGE_SIZE - 1);
    phys = vmm_virt_to_phys(proc->pagemap, page_uva);
    phys &= PTE_PHYS_MASK;
    if (!phys)
        return 0;

    return (void *)(hhdm_base + phys + (uva & (PAGE_SIZE - 1)));
}

int copy_from_user(process_t *proc, void *kdst, uint64_t usrc, size_t n)
{
    uint8_t *d = (uint8_t *)kdst;
    size_t i;

    if (!proc || !kdst)
        return -1;

    for (i = 0; i < n; i++)
    {
        uint8_t *s = (uint8_t *)user_kptr(proc, usrc + i);
        if (!s)
            return -1;
        d[i] = *s;
    }
    return 0;
}

int copy_to_user(process_t *proc, uint64_t udst, const void *ksrc, size_t n)
{
    const uint8_t *s = (const uint8_t *)ksrc;
    size_t i;

    if (!proc || !ksrc)
        return -1;

    for (i = 0; i < n; i++)
    {
        uint8_t *d = (uint8_t *)user_kptr(proc, udst + i);
        if (!d)
            return -1;
        *d = s[i];
    }
    return 0;
}

int copy_user_str(process_t *proc, char *kdst, uint64_t usrc, size_t max)
{
    size_t i;

    if (!proc || !kdst || max == 0)
        return -1;

    for (i = 0; i < max; i++)
    {
        uint8_t *s = (uint8_t *)user_kptr(proc, usrc + i);
        if (!s)
            return -1;
        kdst[i] = (char)*s;
        if (kdst[i] == '\0')
            return 0;
    }
    kdst[max - 1] = '\0';
    return 0;
}
