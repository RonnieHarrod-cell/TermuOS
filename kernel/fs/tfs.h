#pragma once
#include "vfs.h"
#include <stdint.h>

// ─── TFS on-disk layout ───────────────────────────────────────────────────────
//
//  LBA 0       : Superblock      (1 sector)
//  LBA 1..8    : Block bitmap    (8 sectors = 4096 bytes = 32768 block bits)
//  LBA 9..264  : Inode table     (256 sectors = 512 inodes, 32 bytes each)
//  LBA 265+    : Data blocks     (4KB each = 8 sectors)
//
#define TFS_MAGIC 0x54465300 // "TFS\0"
#define TFS_VERSION 1
#define TFS_SECTOR_SIZE 512
#define TFS_BLOCK_SIZE 4096 // 8 sectors
#define TFS_BLOCK_SECTORS (TFS_BLOCK_SIZE / TFS_SECTOR_SIZE)
#define TFS_MAX_INODES 512
#define TFS_INODE_SIZE 64
#define TFS_NUM_DIRECT 9
#define TFS_MAX_DIRECT 10                                     // direct block pointers per inode
#define TFS_INDIRECT_PTRS (TFS_BLOCK_SIZE / sizeof(uint32_t)) // 1024
#define TFS_NAME_MAX 48

#define TFS_LBA_SUPER 0
#define TFS_LBA_BITMAP 1
#define TFS_BITMAP_SECTORS 8
#define TFS_LBA_INODES (TFS_LBA_BITMAP + TFS_BITMAP_SECTORS)
#define TFS_INODE_SECTORS ((TFS_MAX_INODES * TFS_INODE_SIZE) / TFS_SECTOR_SIZE)
#define TFS_LBA_DATA (TFS_LBA_INODES + TFS_INODE_SECTORS)

#define TFS_TYPE_FREE 0
#define TFS_TYPE_FILE 1
#define TFS_TYPE_DIR 2

// Root inode is always inode 0
#define TFS_ROOT_INODE 0

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t total_inodes;
    uint32_t free_inodes;
    uint8_t _pad[492]; // pad to 512 bytes
} __attribute__((packed)) tfs_super_t;

typedef struct
{
    uint8_t type; // TFS_TYPE_*
    uint8_t _pad0[3];
    uint32_t size; // file size in bytes
    uint32_t nlinks;
    uint32_t blocks[TFS_MAX_DIRECT]; // data block indices (0 = unused)
    uint8_t _pad1[TFS_INODE_SIZE - 4 - 4 - 4 - TFS_MAX_DIRECT * 4];
} __attribute__((packed)) tfs_inode_t;

// Directory entry stored in data blocks
#define TFS_DIRENT_SIZE 64
typedef struct
{
    uint32_t inode; // 0 = free slot
    char name[TFS_DIRENT_SIZE - 4];
} __attribute__((packed)) tfs_dirent_t;

#define TFS_DIRENTS_PER_BLOCK (TFS_BLOCK_SIZE / TFS_DIRENT_SIZE)

// ─── Public API ───────────────────────────────────────────────────────────────

int tfs_mount(void); // detect + mount TFS, returns 0 on success
vfs_node_t *tfs_get_root(void);

// Host-side format function (also callable from kernel for mkfs)
// Returns 0 on success
int tfs_format(uint32_t total_blocks);