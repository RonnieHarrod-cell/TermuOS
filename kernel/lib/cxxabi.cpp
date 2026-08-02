#include <stddef.h>
#include <stdint.h>

#include "cxxabi.h"

extern "C"
{
#include "mm/heap.h"
#include "printf.h"
}

void *operator new(size_t size) { return kmalloc(size); }
void *operator new[](size_t size) { return kmalloc(size); }

void operator delete(void *ptr) noexcept { kfree(ptr); }
void operator delete[](void *ptr) noexcept { kfree(ptr); }

void operator delete(void *ptr, size_t) noexcept { kfree(ptr); }
void operator delete[](void *ptr, size_t) noexcept { kfree(ptr); }

extern "C" void __cxa_pure_virtual()
{
    kprintf("cxx: pure virtual function call!\n");
    for (;;)
        __asm__ volatile("cli; hlt");
}

extern "C" int __cxa_guard_acquire(uint64_t *guard)
{
    return !(*(uint8_t *)guard);
}
extern "C" void __cxa_guard_release(uint64_t *guard) { *(uint8_t *)guard = 1; }
extern "C" void __cxa_guard_abort(uint64_t *) {}

extern "C" void *__dso_handle = nullptr;
extern "C" int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }

typedef void (*init_fn_t)(void);
extern "C" init_fn_t __init_array_start[];
extern "C" init_fn_t __init_array_end[];

void cxx_init(void)
{
    for (init_fn_t *fn = __init_array_start; fn != __init_array_end; ++fn)
    {
        (*fn)();
    }
}
