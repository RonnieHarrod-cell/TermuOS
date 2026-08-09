#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "drivers/video/fb.h"
#include "drivers/video/terminal.h"
#include "drivers/input/keyboard.h"
#include "drivers/input/mouse.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/pit.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/heap.h"
#include "sched/scheduler.h"
#include "fs/vfs.h"
#include "fs/ramfs.h"
#include "fs/tfs.h"
#include "drivers/storage/ata.h"
#include "drivers/net/pci.h"
#include "drivers/net/virtio_net.h"
#include "shell/shell.h"
#include "lib/printf.h"
#include "lib/cxxabi.h"
#include "proc/process.h"
#include "ob/object.h"
#include "io/ioman.h"
#include "ipc/port.h"
#include "tlib/tlib_bundle.h"
#include "tlib/exec.h"

#include "luna/luna.h"

LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER

    __attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request fb_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_kernel_address_request kaddr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER

    static uint64_t
    read_cr3(void)
{
    uint64_t cr3;
    __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
    return cr3;
}

void kernel_main(void)
{
    if (!fb_request.response || fb_request.response->framebuffer_count < 1)
        for (;;)
            __asm__ volatile("hlt");

    struct limine_framebuffer *fb = fb_request.response->framebuffers[0];

    fb_init(fb);
    terminal_init();
    terminal_set_offset(0, 0);
    terminal_set_size(fb->width, fb->height);
    fb_clear(0x0D0D0D);

    gdt_init();
    tss_set_kernel_stack(gdt_get_exception_stack());
    idt_init(GDT_KERNEL_CODE);

    kprintf("Initializing Memory...\n");
    pmm_init(memmap_request.response);
    vmm_init(hhdm_request.response->offset, read_cr3());
    heap_init();
    cxx_init();
    keyboard_init();
    mouse_init();

    vfs_init();
    vfs_node_t *root = ramfs_create_root();
    vfs_mount("/", root);
    vfs_mkdir("/etc");
    vfs_mkdir("/home");
    vfs_mkdir("/home/root");
    vfs_mkdir("/bin");
    vfs_mkdir("/mnt");
    int fd = vfs_open("/etc/motd", O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        const char *motd = "Welcome to TermuOS\n";
        vfs_write(fd, motd, 19);
        vfs_close(fd);
    }

    ata_init();

    kprintf("Starting Scheduler...\n");
    ob_init();
    ioman_init();
    ipc_init();
    proc_init();
    scheduler_init();
    pit_init(100);

    ata_ioman_register();
    keyboard_ioman_register();

    if (tfs_mount() == 0)
    {
        vfs_mount("/mnt", tfs_get_root());
        kprintf("tfs: /mnt ready\n");
    }
    else
    {
        kprintf("tfs: no disk or unformatted — run 'mkfs' to format\n");
    }

    pci_init();
    virtio_net_init();

    luna_run();

    process_t *shell_proc = proc_create("shell");
    if (!shell_proc)
        shell_proc = proc_kernel();

    thread_t *shell_thread = thread_create("shell", shell_thread_entry, shell_proc);
    if (!shell_thread)
        kprintf("kernel: failed to create shell thread\n");

    scheduler_yield();
    for (;;)
        __asm__ volatile("hlt");
}