#include "shell.h"
#include "../drivers/video/terminal.h"
#include "../drivers/input/keyboard.h"
#include "../drivers/net/virtio_net.h"
#include "../mm/pmm.h"
#include "../mm/heap.h"
#include "../arch/x86_64/pit.h"
#include "../sched/scheduler.h"
#include "../fs/vfs.h"
#include "../fs/tfs.h"
#include "../net/net.h"
#include "../user/syscall.h"
#include "../proc/process.h"
#include "../tlib/tlib_bundle.h"
#include "../ob/object.h"
#include "../lib/printf.h"
#include "../lib/string.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define INPUT_MAX 256
#define MAX_ARGS 16
#define HOSTNAME "TermuOS"
#define USERNAME "root"

#define VFS_MAX_DEPTH 32

static void normalize_path(const char *raw, char *out, int max)
{
    int starts[VFS_MAX_DEPTH];
    int lens[VFS_MAX_DEPTH];
    int depth = 0;

    const char *p = raw;
    while (*p)
    {
        while (*p == '/')
            p++;
        if (!*p)
            break;

        const char *start = p;
        int len = 0;
        while (*p && *p != '/')
        {
            p++;
            len++;
        }

        if (len == 1 && start[0] == '.')
        {
            continue;
        }
        else if (len == 2 && start[0] == '.' && start[1] == '.')
        {
            if (depth > 0)
                depth--;
        }
        else if (depth < VFS_MAX_DEPTH)
        {
            starts[depth] = (int)(start - raw);
            lens[depth] = len;
            depth++;
        }
    }

    int oi = 0;
    out[oi++] = '/';
    for (int i = 0; i < depth; i++)
    {
        if (i > 0 && oi < max - 1)
            out[oi++] = '/';
        for (int j = 0; j < lens[i] && oi < max - 1; j++)
            out[oi++] = raw[starts[i] + j];
    }
    out[oi] = '\0';
}

static char cwd[VFS_PATH_MAX] = "/";

static inline void outb(uint16_t p, uint8_t v) { __asm__ volatile("outb %0,%1" ::"a"(v), "Nd"(p)); }

const char *shell_get_cwd(void)
{
    return cwd;
}

static void uint_to_string(uint64_t value, char *buffer)
{
    char temp[32];
    int i = 0;
    int j = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (value > 0)
    {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
    {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}

static int sh_strlen(const char *s)
{
    int n = 0;
    while (s[n])
        n++;
    return n;
}
static int sh_atoi(const char *s)
{
    int value = 0;
    int sign = 1;
    if (*s == '-')
    {
        sign = -1;
        s++;
    }
    while (*s >= '0' && *s <= '9')
    {
        value = value * 10 + (*s - '0');
        s++;
    }
    return value * sign;
}
static int sh_strcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b)
    {
        a++;
        b++;
    }
    return *a - *b;
}
static void sh_strcpy(char *d, const char *s, int m)
{
    int i = 0;
    while (s[i] && i < m - 1)
    {
        d[i] = s[i];
        i++;
    }
    d[i] = 0;
}
static void sh_memset(void *p, uint8_t v, int n)
{
    uint8_t *b = p;
    while (n--)
        *b++ = v;
}

static void resolve_path(const char *arg, char *out, int max)
{
    static char raw[VFS_PATH_MAX];
    if (arg[0] == '/')
    {
        sh_strcpy(raw, arg, VFS_PATH_MAX);
    }
    else
    {
        int cl = sh_strlen(cwd);
        sh_strcpy(raw, cwd, VFS_PATH_MAX);
        if (cwd[cl - 1] != '/')
        {
            raw[cl] = '/';
            raw[cl + 1] = 0;
        }
        int ol = sh_strlen(raw);
        int i = 0;
        while (arg[i] && ol + i < VFS_PATH_MAX - 1)
        {
            raw[ol + i] = arg[i];
            i++;
        }
        raw[ol + i] = 0;
    }
    normalize_path(raw, out, max);
}

static int readline(char *buf, int max)
{
    int len = 0;

    while (1)
    {
        char c = keyboard_getchar();

        // ignore empty/no-key values
        if (c == 0)
            continue;

        if (c == '\r')
            continue;

        if (c == '\n')
        {
            terminal_putchar('\n');
            buf[len] = 0;
            return len;
        }

        if (c == '\b')
        {
            if (len > 0)
            {
                len--;

                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }

            continue;
        }

        if (len < max - 1)
        {
            buf[len++] = c;
            terminal_putchar(c);
        }
    }
}

static int parse_args(char *line, char **argv)
{
    int argc = 0;
    char *p = line;
    while (*p)
    {
        while (*p == ' ')
            p++;
        if (!*p)
            break;
        argv[argc++] = p;
        if (argc >= MAX_ARGS)
            break;
        while (*p && *p != ' ')
            p++;
        if (*p)
            *p++ = 0;
    }
    argv[argc] = NULL;
    return argc;
}

// ─── Commands ─────────────────────────────────────────────────────────────────

static void cmd_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("Commands: help clear echo uname uptime mem threads\n");
    kprintf("          ls cd pwd cat write touch mkdir rm reboot shutdown\n");
    kprintf("          exec ps\n");
    kprintf("          ifconfig ping arp smtp\n");
    kprintf("          obdir\n");
}

static void cmd_clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("%c", '\014');
}

static void cmd_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        kprintf("%s", argv[i]);
        if (i < argc - 1)
            terminal_putchar(' ');
    }
    kprintf("\n");
}

static void cmd_uname(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("TermuOS 0.1.0 x86_64\n");
}

static void cmd_uptime(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    uint64_t uptime = syscall_dispatch(SYS_UPTIME, 0, 0, 0, 0, 0, 0);

    char buf[32];
    uint_to_string(uptime, buf);

    kprintf("Uptime ticks: ");
    kprintf(buf);
    kprintf("\n");
}

static void cmd_mem(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    size_t f = pmm_free_pages(), t = pmm_total_pages(), u = t - f;
    kprintf("Total:%uMB Used:%uMB Free:%uMB\n",
            (t * 4096) / (1024 * 1024), (u * 4096) / (1024 * 1024), (f * 4096) / (1024 * 1024));
}

static void cmd_threads(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    extern thread_t threads[MAX_THREADS];
    static const char *st[] = {"dead", "ready", "running", "blocked"};
    kprintf("ID  PID  STATE    NAME\n");
    kprintf("--  ---  -------  ----------------\n");
    for (int i = 0; i < MAX_THREADS; i++)
    {
        if (threads[i].state == THREAD_DEAD)
            continue;
        kprintf("%u\t%u\t%s\t%s\n",
                threads[i].id,
                threads[i].owner ? threads[i].owner->pid : 0,
                st[threads[i].state],
                threads[i].name);
    }
}

static void cmd_pwd(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("%s\n", cwd);
}

static void cmd_cd(int argc, char **argv)
{
    static char path[VFS_PATH_MAX];
    if (argc < 2)
    {
        sh_strcpy(cwd, "/", VFS_PATH_MAX);
        return;
    }
    resolve_path(argv[1], path, VFS_PATH_MAX);
    uint32_t type;
    if (vfs_stat(path, &type, NULL) < 0)
    {
        kprintf("cd: %s: not found\n", argv[1]);
        return;
    }
    if (type != VFS_DIR)
    {
        kprintf("cd: %s: not a directory\n", argv[1]);
        return;
    }
    sh_strcpy(cwd, path, VFS_PATH_MAX);
}

static void cmd_ls(int argc, char **argv)
{
    static char path[VFS_PATH_MAX];
    if (argc < 2)
        sh_strcpy(path, cwd, VFS_PATH_MAX);
    else
        resolve_path(argv[1], path, VFS_PATH_MAX);
    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0)
    {
        kprintf("ls: %s: not found\n", path);
        return;
    }
    char name[VFS_NAME_MAX];
    uint32_t idx = 0;
    while (vfs_readdir(fd, idx++, name) == 0)
    {
        static char full[VFS_PATH_MAX];
        int pl = sh_strlen(path);
        sh_strcpy(full, path, VFS_PATH_MAX);
        if (full[pl - 1] != '/')
        {
            full[pl] = '/';
            full[pl + 1] = 0;
            pl++;
        }
        int ni = 0;
        while (name[ni] && pl + ni < VFS_PATH_MAX - 1)
        {
            full[pl + ni] = name[ni];
            ni++;
        }
        full[pl + ni] = 0;
        uint32_t type = VFS_FILE;
        uint64_t size = 0;
        vfs_stat(full, &type, &size);
        if (type == VFS_DIR)
            kprintf("%s/\n", name);
        else
            kprintf("%s  (%u bytes)\n", name, size);
    }
    vfs_close(fd);
}

static void cmd_cat(int argc, char **argv)
{
    if (argc < 2)
    {
        kprintf("cat: missing filename\n");
        return;
    }
    static char path[VFS_PATH_MAX];
    resolve_path(argv[1], path, VFS_PATH_MAX);
    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0)
    {
        kprintf("cat: %s: not found\n", argv[1]);
        return;
    }
    uint8_t buf[256];
    int n;
    while ((n = vfs_read(fd, buf, sizeof(buf) - 1)) > 0)
    {
        buf[n] = 0;
        for (int i = 0; i < n; i++)
            kprintf("%c", buf[i]); // char by char instead of %s
    }
    kprintf("\n");
    vfs_close(fd);
}

static void cmd_write(int argc, char **argv)
{
    if (argc < 3)
    {
        kprintf("write: usage: write <file> <text>\n");
        return;
    }
    static char path[VFS_PATH_MAX];
    resolve_path(argv[1], path, VFS_PATH_MAX);
    int fd = vfs_open(path, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0)
    {
        kprintf("write: cannot open %s\n", argv[1]);
        return;
    }
    for (int i = 2; i < argc; i++)
    {
        vfs_write(fd, argv[i], sh_strlen(argv[i]));
        if (i < argc - 1)
            vfs_write(fd, " ", 1);
    }
    vfs_write(fd, "\n", 1);
    vfs_close(fd);
}

static void cmd_touch(int argc, char **argv)
{
    if (argc < 2)
    {
        kprintf("touch: missing filename\n");
        return;
    }
    static char path[VFS_PATH_MAX];
    resolve_path(argv[1], path, VFS_PATH_MAX);
    if (vfs_create(path) < 0)
        kprintf("touch: failed\n");
}

static void cmd_mkdir(int argc, char **argv)
{
    if (argc < 2)
    {
        kprintf("mkdir: missing dirname\n");
        return;
    }
    static char path[VFS_PATH_MAX];
    resolve_path(argv[1], path, VFS_PATH_MAX);
    if (vfs_mkdir(path) < 0)
        kprintf("mkdir: failed\n");
}

static void cmd_rm(int argc, char **argv)
{
    if (argc < 2)
    {
        kprintf("rm: missing filename\n");
        return;
    }
    static char path[VFS_PATH_MAX];
    resolve_path(argv[1], path, VFS_PATH_MAX);
    if (vfs_unlink(path) < 0)
        kprintf("rm: %s: not found\n", argv[1]);
}

static inline void outw(uint16_t p, uint16_t v) { __asm__ volatile("outw %0,%1" ::"a"(v), "Nd"(p)); }

static void cmd_shutdown(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("Shutting down...\n");
    // QEMU/VirtualBox ACPI sleep
    outw(0x604, 0x2000);
    // Bochs/Older QEMU
    outw(0xB004, 0x2000);
    // QEMU debug exit
    outb(0x501, 0x31);

    for (;;)
        __asm__ volatile("hlt");
}

static void cmd_reboot(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("Rebooting...\n");
    uint8_t v = 0;
    while (v & 0x02)
        __asm__ volatile("inb $0x64,%0" : "=a"(v));
    __asm__ volatile("outb %0,$0x64" ::"a"((uint8_t)0xfe));
    for (;;)
        __asm__ volatile("hlt");
}

// ─── Network commands ─────────────────────────────────────────────────────────
static void cmd_ifconfig(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("eth0: MAC " MAC_FMT "\n", MAC_ARGS(netif.mac));
    kprintf("      IP  " IP_FMT "\n", IP_ARGS(netif.ip));
    kprintf("      GW  " IP_FMT "\n", IP_ARGS(netif.gateway));
}

static void parse_ip_arg(const char *s, ip4_t *dst)
{
    int octet = 0, val = 0;
    dst->b[0] = dst->b[1] = dst->b[2] = dst->b[3] = 0;
    while (*s)
    {
        if (*s >= '0' && *s <= '9')
            val = val * 10 + (*s - '0');
        else if (*s == '.')
        {
            dst->b[octet++] = val;
            val = 0;
        }
        s++;
    }
    dst->b[octet] = val;
}

static void cmd_ping(int argc, char **argv)
{
    if (argc < 2)
    {
        kprintf("ping: usage: ping <ip>\n");
        return;
    }
    ip4_t dst = {0};
    parse_ip_arg(argv[1], &dst);
    kprintf("ping: " IP_FMT "\n", IP_ARGS(dst));
    net_send_icmp_echo(dst, 1, 1); /* sends ARP if needed */
    /* poll RX while waiting - IRQ may not fire without MSI-X */
    for (volatile int i = 0; i < 500; i++)
    {
        virtio_net_poll();
        for (volatile int j = 0; j < 100000; j++)
            ;
    }
    net_send_icmp_echo(dst, 1, 2);
}

static void cmd_arp(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("arp: requesting gateway " IP_FMT "\n", IP_ARGS(netif.gateway));
    net_send_arp_request(netif.gateway);
}

static void cmd_smtp(int argc, char **argv)
{
    if (argc < 6)
    {
        kprintf("smtp: usage: smtp <server-ip> [port] <from> <to> <subject> <body>\n");
        return;
    }

    ip4_t dst = {0};
    parse_ip_arg(argv[1], &dst);

    int port = 587;
    const char *from;
    const char *to;
    const char *subject;
    int body_start;

    if (argc == 6)
    {
        from = argv[2];
        to = argv[3];
        subject = argv[4];
        body_start = 5;
    }
    else
    {
        port = sh_atoi(argv[2]);
        if (port <= 0)
        {
            kprintf("smtp: invalid port %s\n", argv[2]);
            return;
        }
        from = argv[3];
        to = argv[4];
        subject = argv[5];
        body_start = 6;
    }

    char body[160];
    body[0] = '\0';
    int off = 0;
    for (int i = body_start; i < argc; i++)
    {
        int len = sh_strlen(argv[i]);
        for (int j = 0; j < len && off < (int)sizeof(body) - 2; j++)
            body[off++] = argv[i][j];
        if (i < argc - 1 && off < (int)sizeof(body) - 1)
            body[off++] = ' ';
    }
    body[off] = '\0';

    kprintf("smtp: sending plain SMTP payload to %s:%d\n", argv[1], port);
    kprintf("smtp: modern providers like Gmail usually require TLS/auth; this build does not implement that yet\n");
    net_send_smtp(dst, (uint16_t)port, "termuos", from, to, subject, body);
    for (volatile int i = 0; i < 500; i++)
    {
        virtio_net_poll();
        for (volatile int j = 0; j < 100000; j++)
            ;
    }
    kprintf("smtp: transmit attempt complete; any server replies will be shown above if received\n");
}

// ─── PID ─────────────────────────────────────────────────────────────────

static void cmd_pid(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    uint64_t pid = syscall_dispatch(SYS_GETPID, 0, 0, 0, 0, 0, 0);

    kprintf("PID: ");

    char buf[32];
    uint_to_string(pid, buf);

    kprintf(buf);
    kprintf("\n");
}

static void cmd_sleep(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    kprintf("Sleeping...\n");

    syscall_dispatch(SYS_SLEEP, 100, 0, 0, 0, 0, 0);

    kprintf("Awake!\n");
}

static void cmd_yield(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    kprintf("Yielding CPU...\n");

    syscall_dispatch(SYS_YIELD, 0, 0, 0, 0, 0, 0);

    kprintf("Returned from yield\n");
}

// ─── Exec ─────────────────────────────────────────────────────────────────────

static void cmd_mkfs(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("mkfs: formatting disk as TFS...\n");
    if (tfs_format(8192) < 0)
    {
        kprintf("mkfs: format failed\n");
        return;
    }
    if (tfs_mount() < 0)
    {
        kprintf("mkfs: formatted but mount failed\n");
        return;
    }
    vfs_mount("/mnt", tfs_get_root());
    kprintf("mkfs: done — disk mounted at /mnt\n");
}

static void cmd_exec(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    kprintf("exec: ELF loader not available in this build\n");
}

static void cmd_ps(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    proc_list();
}

// ─── Object ─────────────────────────────────────────────────────────────────
static void cmd_obdir(int argc, char **argv)
{
    const char *path = (argc >= 2) ? argv[1] : "\\";
    ob_list(path);
}

static void cmd_run(int argc, char **argv)
{
    if (argc < 2)
    {
        kprintf("run: usage: run <path/to/App.tapp>\n");
        return;
    }

    static char path[VFS_PATH_MAX];
    resolve_path(argv[1], path, VFS_PATH_MAX);

    int plen = sh_strlen(path);
    int has_ext = (plen > 5 &&
                   path[plen - 5] == '.' &&
                   path[plen - 4] == 't' &&
                   path[plen - 3] == 'a' &&
                   path[plen - 2] == 'p' &&
                   path[plen - 1] == 'p');
    if (!has_ext && plen + 5 < VFS_PATH_MAX)
    {
        path[plen + 0] = '.';
        path[plen + 1] = 't';
        path[plen + 2] = 'a';
        path[plen + 3] = 'p';
        path[plen + 4] = 'p';
        path[plen + 5] = '\0';
    }

    // check exists
    uint32_t type;
    if (vfs_stat(path, &type, NULL) < 0)
    {
        kprintf("run: %s: not found\n", path);
        return;
    }
    if (type != VFS_DIR)
    {
        kprintf("run: %s: not a .tapp bundle (expected a directory)\n", path);
        return;
    }

    // load and launch
    static tlib_app_t app;
    if (tlib_bundle_load(path, &app) < 0)
    {
        kprintf("run: failed to load bundle %s\n", path);
        return;
    }
    kprintf("run: launching %s v%s\n", app.manifest.name, app.manifest.version);
    if (tlib_bundle_launch(&app) < 0)
    {
        kprintf("run: launch failed\n");
    }
    // returns immediatly; new thread is scheduled independently
}

// ─── Dispatch ─────────────────────────────────────────────────────────────────

typedef struct
{
    const char *name;
    void (*fn)(int, char **);
} command_t;
static const command_t commands[] = {
    {"help", cmd_help},
    {"clear", cmd_clear},
    {"echo", cmd_echo},
    {"uname", cmd_uname},
    {"uptime", cmd_uptime},
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"pwd", cmd_pwd},
    {"cd", cmd_cd},
    {"ls", cmd_ls},
    {"cat", cmd_cat},
    {"write", cmd_write},
    {"touch", cmd_touch},
    {"mkdir", cmd_mkdir},
    {"rm", cmd_rm},
    {"reboot", cmd_reboot},
    {"shutdown", cmd_shutdown},
    {"ifconfig", cmd_ifconfig},
    {"ping", cmd_ping},
    {"arp", cmd_arp},
    {"smtp", cmd_smtp},
    {"pid", cmd_pid},
    {"sleep", cmd_sleep},
    {"yield", cmd_yield},
    {"exec", cmd_exec},
    {"mkfs", cmd_mkfs},
    {"obdir", cmd_obdir},
    {"run", cmd_run},
    {"ps", cmd_ps},
    {NULL, NULL}};

static void dispatch(char *line)
{
    if (!line || !line[0])
        return;
    char *argv[MAX_ARGS];
    int argc = parse_args(line, argv);
    if (!argc)
        return;
    for (int i = 0; commands[i].name; i++)
        if (sh_strcmp(argv[0], commands[i].name) == 0)
        {
            commands[i].fn(argc, argv);
            return;
        }
    kprintf("tsh: command not found: %s\n", argv[0]);
}

static void print_prompt(void)
{
    kprintf(USERNAME "@");
    kprintf(HOSTNAME);
    kprintf(":");
    kprintf(cwd);
    kprintf("# ");
}

void shell_run(void)
{
    char input[INPUT_MAX];
    kprintf("\nTermuOS 0.1.0 -- type 'help' for commands.\n\n");
    while (1)
    {
        print_prompt();
        sh_memset(input, 0, sizeof(input));
        readline(input, INPUT_MAX);
        dispatch(input);
    }
}

void shell_run_command(const char *line)
{
    static char buf[INPUT_MAX];
    int i = 0;
    while (line[i] && i < INPUT_MAX - 1)
    {
        buf[i] = line[i];
        i++;
    }
    buf[i] = '\0';
    dispatch(buf);
}

void shell_thread_entry(void)
{
    shell_run();
    thread_exit();
}
