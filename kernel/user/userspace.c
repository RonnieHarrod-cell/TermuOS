#include "userspace.h"
#include "syscall.h"
#include "../arch/x86_64/gdt.h"
#include "../lib/printf.h"
#include "../proc/process.h"
#include "../sched/scheduler.h"
#include <stdint.h>

void userspace_init(void)
{
    syscall_init();
}

void jump_userspace(uint64_t entry, uint64_t stack)
{
    __asm__ volatile("cli");

    process_t *proc = thread_current()->owner;
    if (proc && proc->pagemap)
        vmm_switch(proc->pagemap);

    tss_set_kernel_stack(gdt_get_exception_stack());
    enter_userspace(entry, stack);
}