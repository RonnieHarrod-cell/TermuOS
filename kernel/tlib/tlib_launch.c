#include "tlib_bundle.h"
#include "exec.h"
#include "../proc/process.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"
#include "../sched/scheduler.h"
#include "../user/userspace.h"
#include "../ipc/port.h"
#include "../lib/printf.h"
#include <stdint.h>

// Launch a bundled app
//
// Steps:
//   1. Confirm that the app was loaded.
//   2. Create a kernel process_t.
//   3. Load the entry binary into the process's pagemap.
//   4. Register the app's declared ports under \App\<bundle_id>\<port_name>.
//   5. Switch the pagemap and jump to userspace. This does not return on success.
// -----------------------------------------------------------------------------

// Build a string of the form prefix + suffix into dst[max].
static void tl_build_path(char *dst, int max,
                          const char *prefix, const char *suffix)
{
  int i = 0, j = 0;
  while (prefix[j] && i < max - 1)
    dst[i++] = prefix[j++];
  j = 0;
  while (suffix[j] && i < max - 1)
    dst[i++] = suffix[j++];
  dst[i] = '\0';
}

static void tlib_app_thread_entry(void)
{
  // Retrieve the launch context stored in the owning process object.
  thread_t *self = thread_current();
  tlib_launch_ctx_t *ctx = (tlib_launch_ctx_t *)self->owner->ob_header->body;

  tlib_set_perm_mask(ctx->perm_mask);
  vmm_switch(ctx->pagemap);
  jump_userspace(ctx->entry, ctx->stack_top);

  // This should never return.
  kprintf("tlib: thread entry returned\n");
  thread_exit();
}

int tlib_bundle_launch(tlib_app_t *app)
{
  if (!app || !app->loaded)
  {
    kprintf("tlib: launch called on unloaded app\n");
    return -1;
  }

  tlib_manifest_t *m = &app->manifest;

  // Step 1: create the process.
  process_t *proc = proc_create(m->name);
  if (!proc)
  {
    kprintf("tlib: could not create process for '%s'\n", m->name);
    return -1;
  }

  // Step 2: load the ELF into the process pagemap.
  uint64_t entry = 0;
  if (exec_load(app->entry_path, proc, &entry) != 0)
  {
    kprintf("tlib: exec_load failed for '%s'\n", app->entry_path);
    proc_exit(proc, -1);
    return -1;
  }

  // Step 3: register the declared ports in the namespace.
  // This creates \App\<bundle_id> and one port per entry in ports[].
  // The ports are registered early so other apps can discover them before the new process runs its first instruction.
  {
    ob_mkdir("\\App");
    char ns_dir[128];
    tl_build_path(ns_dir, sizeof(ns_dir), "\\App\\", m->bundle_id);
    ob_mkdir(ns_dir);

    for (int i = 0; i < m->port_count; i++)
    {
      port_t *p = port_create(m->ports[i]);
      if (!p)
      {
        kprintf("tlib: failed to create port '%s'\n", m->ports[i]);
        // This is non-fatal, so continue with the remaining ports.
        continue;
      }

      // Insert the port into \App\<bundle_id>\<port_name>.
      char ns_port[192];
      int dlen = 0;
      while (ns_dir[dlen])
        dlen++;
      int si = 0;
      while (ns_dir[si] && si < (int)sizeof(ns_port) - 1)
      {
        ns_port[si] = ns_dir[si];
        si++;
      }
      ns_port[si++] = '\\';
      const char *pn = m->ports[i];
      int pi2 = 0;
      while (pn[pi2] && si + pi2 < (int)sizeof(ns_port) - 1)
      {
        ns_port[si + pi2] = pn[pi2];
        pi2++;
      }
      ns_port[si + pi2] = '\0';

      ob_insert(ns_port, p->ob_header);
      kprintf("tlib: registered port '%s'\n", ns_port);
    }
  }

  // Step 4: switch the pagemap and jump to userspace.
  tlib_launch_ctx_t *ctx = (tlib_launch_ctx_t *)kmalloc(sizeof(tlib_launch_ctx_t));
  if (!ctx)
  {
    kprintf("tlib: OOM allocating lauch ctx\n");
    proc_exit(proc, -1);
    return -1;
  }

  ctx->entry = entry;
  ctx->stack_top = EXEC_USER_STACK_TOP;
  ctx->pagemap = proc->pagemap;
  ctx->perm_mask = m->perm_mask;

  // Store the launch context in the process object so the thread entry can find it.
  proc->ob_header->body = ctx;

  thread_t *t = thread_create(m->name, tlib_app_thread_entry, proc);
  if (!t)
  {
    kprintf("tlib: failed to create thread for '%s'\n", m->name);
    kfree(ctx);
    proc_exit(proc, -1);
    return -1;
  }

  kprintf("tlib: scheduled '%s' (pid %u, tid %u)\n", m->name, proc->pid, (uint32_t)t->id);
  return 0;
  // The caller returns normally; the scheduler will pick up the new thread on the next tick.
}
