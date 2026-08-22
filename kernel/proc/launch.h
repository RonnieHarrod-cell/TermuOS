#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../mm/vmm.h"

#define PERM_IPC_SEND (1u << 0)
#define PERM_IPC_RECEIVE (1u << 1)
#define PERM_FS_READ (1u << 2)
#define PERM_FS_WRITE (1u << 3)
#define PERM_PROC_SPAWN (1u << 4)

typedef struct
{
    uint64_t entry;
    uint64_t stack_top;
    pagemap_t pagemap;
    uint32_t perm_mask;
} launch_ctx_t;

int exec_launch(const char *vfs_path, uint32_t perm_mask);
int exec_launch_args(const char *vfs_path, uint32_t perm_mask,
                     int argc, char *const argv[]);

int proc_check_perm(uint32_t perm);

void proc_set_perm_mask(uint32_t mask);
