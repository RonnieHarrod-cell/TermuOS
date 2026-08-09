#include "launch.h"
#include "exec.h"
#include "process.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"
#include "../sched/scheduler.h"
#include "../user/userspace.h"
#include "../lib/printf.h"
#include <stdint.h>

static void exec_thread_entry()
{
  thread_t *self = thread_current();
  launch_ctx_t *ctx = (launch_ctx_t *)self->owner->ob_header->body;

  kprintf("exec: thread running, entry=0x%x stack=0x%x pm=0x%x\n",
          ctx->entry, ctx->stack_top, (uint64_t)ctx->pagemap);

  proc_set_perm_mask(ctx->perm_mask);

  kprintf("exec: about to vmm_switch\n");
  vmm_switch(ctx->pagemap);

  kprintf("exec: about to jump_userspace\n");
  jump_userspace(ctx->entry, ctx->stack_top);

  kprintf("exec: thread entry returned\n");
  thread_exit();
}

int exec_launch(const char *vfs_path, uint32_t perm_mask)
{
  if (!vfs_path)
    return -1;

  const char *name = vfs_path;
  for (const char *p = vfs_path; *p; p++)
    if (*p == '/')
      name = p + 1;

  process_t *proc = proc_create(name);
  if (!proc)
  {
    kprintf("exec: could not create process for '%s'\n", vfs_path);
    return -1;
  }

  uint64_t entry = 0;
  if (exec_load(vfs_path, proc, &entry) != 0)
  {
    kprintf("exec: exec_load failed for '%s'\n", vfs_path);
    proc_exit(proc, -1);
    return -1;
  }

  launch_ctx_t *ctx = (launch_ctx_t *)kmalloc(sizeof(launch_ctx_t));
  if (!ctx)
  {
    kprintf("exec: OOM allocating launch ctx\n");
    proc_exit(proc, -1);
    return -1;
  }

  ctx->entry = entry;
  ctx->stack_top = EXEC_USER_STACK_TOP;
  ctx->pagemap = proc->pagemap;
  ctx->perm_mask = perm_mask;

  proc->ob_header->body = ctx;

  thread_t *t = thread_create(name, exec_thread_entry, proc);
  if (!t)
  {
    kprintf("exec: failed to create thread for '%s'\n", vfs_path);
    kfree(ctx);
    proc_exit(proc, -1);
    return -1;
  }

  kprintf("exec: scheduled '%s' (pid %u, tid %u)\n", name, proc->pid, (uint32_t)t->id);
  return 0;
}

static uint32_t current_perm_mask = 0xffffffff; // kernel: all perms

void proc_set_perm_mask(uint32_t mask)
{
  current_perm_mask = mask;
}

int proc_check_perm(uint32_t perm)
{
  if (current_perm_mask == 0xffffffff)
    return 1; // kernel process
  if (current_perm_mask & perm)
    return 1;
  kprintf("proc: permission denied (needed 0x%x, have 0x%x)\n", perm, current_perm_mask);
  return 0;
}
