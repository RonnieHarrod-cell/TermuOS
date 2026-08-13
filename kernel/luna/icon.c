#include "icon.h"
#include <limine.h>

extern volatile struct limine_module_request module_request;

static int ends_with(const char *s, const char *suf)
{
    if (!s || !suf)
        return 0;
    int ls = 0, lt = 0;
    while (s[ls])
        ls++;
    while (suf[lt])
        lt++;
    if (lt > ls)
        return 0;
    for (int i = 0; i < lt; i++)
        if (s[ls - lt + i] != suf[i])
            return 0;
    return 1;
}

int luna_icon_load(const char *name, int w, int h, luna_icon_t *out)
{
    out->w = w;
    out->h = h;
    out->rgba = 0;
    out->size = 0;

    if (!module_request.response)
        return -1;

    size_t need = (size_t)w * (size_t)h * 4;

    for (uint64_t i = 0; i < module_request.response->module_count; i++)
    {
        struct limine_file *f = module_request.response->modules[i];
        if (!f || !f->path || !f->address)
            continue;
        if (!ends_with(f->path, name))
            continue;
        if (f->size < need)
            return -1;
        out->rgba = (const uint8_t *)f->address;
        out->size = (size_t)f->size;
        return 0;
    }
    return -1;
}
