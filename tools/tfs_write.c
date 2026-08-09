/*
 * tools/tfs_write.c
 * Host-side tool: write a host directory tree into a TFS disk.img
 *
 * Build:
 *   cc -O2 -o tools/tfs_write tools/tfs_write.c
 *
 * Usage:
 *   ./tools/tfs_write disk.img apps/HelloGui /mnt/HelloGui
 *
 *   arg1 = disk image path
 *   arg2 = host source directory
 *   arg3 = destination path on TFS (must start with /mnt/)
 *          The kernel mounts TFS root at /mnt, so /mnt/HelloGui
 *          becomes inode 0 child "HelloGui" in TFS root.
 *
 * Must stay in sync with kernel/fs/tfs.h and tools/mkfs_tfs.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* ── TFS constants (mirror of tfs.h) ─────────────────────────── */
#define TFS_MAGIC 0x54465300
#define TFS_SECTOR_SIZE 512
#define TFS_BLOCK_SIZE 4096
#define TFS_BLOCK_SECTORS (TFS_BLOCK_SIZE / TFS_SECTOR_SIZE)
#define TFS_MAX_INODES 512
#define TFS_INODE_SIZE 64
#define TFS_MAX_DIRECT 10
#define TFS_NAME_MAX 48
#define TFS_DIRENT_SIZE 64

#define TFS_LBA_SUPER 0
#define TFS_LBA_BITMAP 1
#define TFS_BITMAP_SECTORS 8
#define TFS_LBA_INODES (TFS_LBA_BITMAP + TFS_BITMAP_SECTORS)
#define TFS_INODE_SECTORS ((TFS_MAX_INODES * TFS_INODE_SIZE) / TFS_SECTOR_SIZE)
#define TFS_LBA_DATA (TFS_LBA_INODES + TFS_INODE_SECTORS)

#define TFS_TYPE_FREE 0
#define TFS_TYPE_FILE 1
#define TFS_TYPE_DIR 2

#define DIRENTS_PER_BLOCK (TFS_BLOCK_SIZE / TFS_DIRENT_SIZE)

typedef struct
{
    uint32_t magic, version, total_blocks, free_blocks;
    uint32_t total_inodes, free_inodes;
    uint8_t _pad[488];
} __attribute__((packed)) tfs_super_t;

typedef struct
{
    uint8_t type;
    uint8_t _pad0[3];
    uint32_t size;
    uint32_t nlinks;
    uint32_t blocks[TFS_MAX_DIRECT];
    uint8_t _pad1[TFS_INODE_SIZE - 4 - 4 - 4 - TFS_MAX_DIRECT * 4];
} __attribute__((packed)) tfs_inode_t;

typedef struct
{
    uint32_t inode;
    char name[TFS_NAME_MAX];
    uint8_t _pad[TFS_DIRENT_SIZE - 4 - TFS_NAME_MAX];
} __attribute__((packed)) tfs_dirent_t;

/* ── global state ─────────────────────────────────────────────── */
static FILE *img;
static tfs_super_t sb;
static uint32_t total_blocks;

/* ── low-level I/O ────────────────────────────────────────────── */
static void read_sector(uint32_t lba, void *buf)
{
    fseek(img, (long)lba * TFS_SECTOR_SIZE, SEEK_SET);
    fread(buf, TFS_SECTOR_SIZE, 1, img);
}
static void write_sector(uint32_t lba, const void *buf)
{
    fseek(img, (long)lba * TFS_SECTOR_SIZE, SEEK_SET);
    fwrite(buf, TFS_SECTOR_SIZE, 1, img);
}

/* ── superblock ───────────────────────────────────────────────── */
static void sb_read(void)
{
    uint8_t buf[TFS_SECTOR_SIZE];
    read_sector(TFS_LBA_SUPER, buf);
    memcpy(&sb, buf, sizeof(sb));
    total_blocks = sb.total_blocks;
}
static void sb_write(void)
{
    uint8_t buf[TFS_SECTOR_SIZE];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &sb, sizeof(sb));
    write_sector(TFS_LBA_SUPER, buf);
}

/* ── bitmap ───────────────────────────────────────────────────── */
static int bitmap_alloc_block(void)
{
    uint8_t bmap[TFS_BITMAP_SECTORS * TFS_SECTOR_SIZE];
    for (int s = 0; s < TFS_BITMAP_SECTORS; s++)
        read_sector(TFS_LBA_BITMAP + s, bmap + s * TFS_SECTOR_SIZE);

    for (uint32_t i = 0; i < total_blocks; i++)
    {
        if (!(bmap[i / 8] & (1 << (i % 8))))
        {
            bmap[i / 8] |= (1 << (i % 8));
            for (int s = 0; s < TFS_BITMAP_SECTORS; s++)
                write_sector(TFS_LBA_BITMAP + s, bmap + s * TFS_SECTOR_SIZE);
            sb.free_blocks--;
            return (int)i;
        }
    }
    return -1; /* disk full */
}

/* Convert data block index → LBA */
static uint32_t block_lba(uint32_t blk)
{
    return TFS_LBA_DATA + blk * TFS_BLOCK_SECTORS;
}

/* ── inode table ──────────────────────────────────────────────── */
static void inode_read(uint32_t ino, tfs_inode_t *out)
{
    uint32_t sector = TFS_LBA_INODES + (ino * TFS_INODE_SIZE) / TFS_SECTOR_SIZE;
    uint32_t offset = (ino * TFS_INODE_SIZE) % TFS_SECTOR_SIZE;
    uint8_t buf[TFS_SECTOR_SIZE];
    read_sector(sector, buf);
    memcpy(out, buf + offset, sizeof(tfs_inode_t));
}
static void inode_write(uint32_t ino, const tfs_inode_t *in)
{
    uint32_t sector = TFS_LBA_INODES + (ino * TFS_INODE_SIZE) / TFS_SECTOR_SIZE;
    uint32_t offset = (ino * TFS_INODE_SIZE) % TFS_SECTOR_SIZE;
    uint8_t buf[TFS_SECTOR_SIZE];
    read_sector(sector, buf);
    memcpy(buf + offset, in, sizeof(tfs_inode_t));
    write_sector(sector, buf);
}
static int inode_alloc(void)
{
    for (uint32_t i = 0; i < TFS_MAX_INODES; i++)
    {
        tfs_inode_t n;
        inode_read(i, &n);
        if (n.type == TFS_TYPE_FREE)
        {
            sb.free_inodes--;
            return (int)i;
        }
    }
    return -1;
}

/* ── directory helpers ────────────────────────────────────────── */

/* Look up name in dir inode → child inode number, or -1 */
static int dir_lookup(uint32_t dir_ino, const char *name)
{
    tfs_inode_t d;
    inode_read(dir_ino, &d);
    uint32_t ndirents = d.size / TFS_DIRENT_SIZE;
    uint32_t checked = 0;
    for (int b = 0; b < TFS_MAX_DIRECT && checked < ndirents; b++)
    {
        if (!d.blocks[b])
            continue;
        uint8_t blk[TFS_BLOCK_SIZE];
        uint32_t lba = block_lba(d.blocks[b]);
        for (int s = 0; s < TFS_BLOCK_SECTORS; s++)
            read_sector(lba + s, blk + s * TFS_SECTOR_SIZE);
        for (int e = 0; e < DIRENTS_PER_BLOCK && checked < ndirents; e++, checked++)
        {
            tfs_dirent_t *de = (tfs_dirent_t *)(blk + e * TFS_DIRENT_SIZE);
            if (de->inode && strncmp(de->name, name, TFS_NAME_MAX) == 0)
                return (int)de->inode;
        }
    }
    return -1;
}

/* Add a dirent (name → child_ino) to dir_ino */
static int dir_add(uint32_t dir_ino, const char *name, uint32_t child_ino)
{
    tfs_inode_t d;
    inode_read(dir_ino, &d);

    uint32_t ndirents = d.size / TFS_DIRENT_SIZE;
    uint32_t slot_block = ndirents / DIRENTS_PER_BLOCK;
    uint32_t slot_entry = ndirents % DIRENTS_PER_BLOCK;

    if (slot_block >= TFS_MAX_DIRECT)
    {
        fprintf(stderr, "dir full\n");
        return -1;
    }

    /* Allocate block if needed */
    if (!d.blocks[slot_block])
    {
        int nb = bitmap_alloc_block();
        if (nb < 0)
        {
            fprintf(stderr, "disk full (blocks)\n");
            return -1;
        }
        d.blocks[slot_block] = (uint32_t)nb;
        /* zero the new block */
        uint8_t zero[TFS_BLOCK_SIZE];
        memset(zero, 0, sizeof(zero));
        uint32_t lba = block_lba(d.blocks[slot_block]);
        for (int s = 0; s < TFS_BLOCK_SECTORS; s++)
            write_sector(lba + s, zero + s * TFS_SECTOR_SIZE);
    }

    /* Read block, write dirent, write block back */
    uint8_t blk[TFS_BLOCK_SIZE];
    uint32_t lba = block_lba(d.blocks[slot_block]);
    for (int s = 0; s < TFS_BLOCK_SECTORS; s++)
        read_sector(lba + s, blk + s * TFS_SECTOR_SIZE);

    tfs_dirent_t *de = (tfs_dirent_t *)(blk + slot_entry * TFS_DIRENT_SIZE);
    memset(de, 0, TFS_DIRENT_SIZE);
    de->inode = child_ino;
    strncpy(de->name, name, TFS_NAME_MAX - 1);

    for (int s = 0; s < TFS_BLOCK_SECTORS; s++)
        write_sector(lba + s, blk + s * TFS_SECTOR_SIZE);

    d.size += TFS_DIRENT_SIZE;
    inode_write(dir_ino, &d);
    return 0;
}

/* mkdir under parent_ino with given name → new inode number */
static int tfs_mkdir(uint32_t parent_ino, const char *name)
{
    int exists = dir_lookup(parent_ino, name);
    if (exists >= 0)
        return exists; /* already there */

    int ino = inode_alloc();
    if (ino < 0)
    {
        fprintf(stderr, "out of inodes\n");
        return -1;
    }

    tfs_inode_t d;
    memset(&d, 0, sizeof(d));
    d.type = TFS_TYPE_DIR;
    d.nlinks = 1;
    inode_write((uint32_t)ino, &d);

    if (dir_add(parent_ino, name, (uint32_t)ino) < 0)
        return -1;
    return ino;
}

/* Write a host file into TFS under parent_ino with given name */
static int tfs_write_file(uint32_t parent_ino, const char *name,
                          const char *host_path)
{
    /* Read host file */
    FILE *f = fopen(host_path, "rb");
    if (!f)
    {
        fprintf(stderr, "cannot open %s\n", host_path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long fsz = ftell(f);
    rewind(f);
    uint8_t *data = malloc((size_t)fsz);
    if (!data)
    {
        fclose(f);
        return -1;
    }
    fread(data, 1, (size_t)fsz, f);
    fclose(f);

    /* Remove existing entry if any */
    int old = dir_lookup(parent_ino, name);
    (void)old; /* overwrite not implemented — skip if exists */
    if (old >= 0)
    {
        printf("  skip (already exists): %s\n", name);
        free(data);
        return 0;
    }

    /* Allocate inode */
    int ino = inode_alloc();
    if (ino < 0)
    {
        fprintf(stderr, "out of inodes\n");
        free(data);
        return -1;
    }

    tfs_inode_t fi;
    memset(&fi, 0, sizeof(fi));
    fi.type = TFS_TYPE_FILE;
    fi.nlinks = 1;
    fi.size = (uint32_t)fsz;

    /* Write data blocks */
    uint32_t written = 0;
    for (int b = 0; b < TFS_MAX_DIRECT && written < (uint32_t)fsz; b++)
    {
        int nb = bitmap_alloc_block();
        if (nb < 0)
        {
            fprintf(stderr, "disk full\n");
            free(data);
            return -1;
        }
        fi.blocks[b] = (uint32_t)nb;

        uint8_t blk[TFS_BLOCK_SIZE];
        memset(blk, 0, sizeof(blk));
        uint32_t chunk = (uint32_t)fsz - written;
        if (chunk > TFS_BLOCK_SIZE)
            chunk = TFS_BLOCK_SIZE;
        memcpy(blk, data + written, chunk);
        written += chunk;

        uint32_t lba = block_lba(fi.blocks[b]);
        for (int s = 0; s < TFS_BLOCK_SECTORS; s++)
            write_sector(lba + s, blk + s * TFS_SECTOR_SIZE);
    }

    inode_write((uint32_t)ino, &fi);
    dir_add(parent_ino, name, (uint32_t)ino);
    free(data);
    printf("  wrote %s (%ld bytes, inode %d)\n", name, fsz, ino);
    return 0;
}

/* Recursively copy host_dir → TFS dir at tfs_parent_ino */
static int copy_dir(const char *host_dir, uint32_t tfs_ino)
{
    DIR *d = opendir(host_dir);
    if (!d)
    {
        fprintf(stderr, "cannot opendir %s\n", host_dir);
        return -1;
    }
    struct dirent *de;
    while ((de = readdir(d)))
    {
        if (de->d_name[0] == '.')
            continue; /* skip . and .. */

        /* Build host path */
        char host_path[4096];
        snprintf(host_path, sizeof(host_path), "%s/%s", host_dir, de->d_name);

        struct stat st;
        if (stat(host_path, &st) < 0)
            continue;

        if (S_ISDIR(st.st_mode))
        {
            printf("  mkdir %s\n", de->d_name);
            int sub_ino = tfs_mkdir(tfs_ino, de->d_name);
            if (sub_ino < 0)
            {
                closedir(d);
                return -1;
            }
            if (copy_dir(host_path, (uint32_t)sub_ino) < 0)
            {
                closedir(d);
                return -1;
            }
        }
        else if (S_ISREG(st.st_mode))
        {
            tfs_write_file(tfs_ino, de->d_name, host_path);
        }
    }
    closedir(d);
    return 0;
}

/* ── main ─────────────────────────────────────────────────────── */
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr,
                "Usage: tfs_write disk.img <host-src-file-or-dir> <tfs-dest-path>\n"
                "Examples:\n"
                "  tfs_write disk.img hello /mnt/hello        (single file)\n"
                "  tfs_write disk.img apps/HelloGui /mnt/HelloGui  (directory tree)\n");
        return 1;
    }
    const char *img_path = argv[1];
    const char *host_src = argv[2];
    const char *tfs_dest = argv[3];

    struct stat src_st;
    if (stat(host_src, &src_st) < 0)
    {
        perror("stat host-src");
        return 1;
    }
    int src_is_dir = S_ISDIR(src_st.st_mode);

    img = fopen(img_path, "r+b");
    if (!img)
    {
        perror("open disk.img");
        return 1;
    }

    sb_read();
    if (sb.magic != TFS_MAGIC)
    {
        fprintf(stderr, "Not a TFS image (bad magic)\n");
        fclose(img);
        return 1;
    }

    /* Parse tfs_dest: strip leading /mnt/ and walk/create the path
     * under TFS root (inode 0).                                    */
    const char *rel = tfs_dest;
    if (rel[0] == '/')
        rel++;
    /* skip "mnt/" prefix if present */
    if (rel[0] == 'm' && rel[1] == 'n' && rel[2] == 't' && rel[3] == '/')
        rel += 4;

    /* Walk all-but-the-last path component as intermediate directories,
     * creating them as needed.                                        */
    uint32_t cur_ino = 0; /* TFS root */
    char seg[TFS_NAME_MAX];
    char last_seg[TFS_NAME_MAX];
    last_seg[0] = '\0';
    const char *p = rel;
    while (*p)
    {
        const char *slash = p;
        while (*slash && *slash != '/')
            slash++;
        int len = (int)(slash - p);
        if (len == 0)
        {
            p = slash + 1;
            continue;
        }
        if (len >= TFS_NAME_MAX)
        {
            fprintf(stderr, "path segment too long\n");
            return 1;
        }
        memcpy(seg, p, len);
        seg[len] = '\0';

        if (*slash == '\0')
        {
            /* Last component. */
            memcpy(last_seg, seg, len + 1);
            break;
        }
        else
        {
            /* Intermediate directory */
            int sub = dir_lookup(cur_ino, seg);
            if (sub < 0)
                sub = tfs_mkdir(cur_ino, seg);
            if (sub < 0)
                return 1;
            cur_ino = (uint32_t)sub;
        }
        p = slash + 1;
    }

    if (src_is_dir)
    {
        /* Directory tree copy: the last component becomes a TFS directory,
         * then we recursively copy host_src's contents into it.          */
        printf("Creating TFS directory: %s\n", last_seg);
        int bino = tfs_mkdir(cur_ino, last_seg);
        if (bino < 0)
            return 1;
        cur_ino = (uint32_t)bino;

        printf("Copying %s → TFS:%s (inode %u)\n", host_src, tfs_dest, cur_ino);
        if (copy_dir(host_src, cur_ino) < 0)
        {
            fprintf(stderr, "copy failed\n");
            fclose(img);
            return 1;
        }
    }
    else
    {
        /* Single file copy: write host_src directly as last_seg inside
         * the resolved parent directory.                                */
        printf("Copying %s → TFS:%s\n", host_src, tfs_dest);
        if (tfs_write_file(cur_ino, last_seg, host_src) < 0)
        {
            fprintf(stderr, "copy failed\n");
            fclose(img);
            return 1;
        }
    }

    sb_write();
    fclose(img);
    printf("Done.\n");
    return 0;
}