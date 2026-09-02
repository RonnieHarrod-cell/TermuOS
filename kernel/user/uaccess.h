#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../proc/process.h"

/*
 * Safe kernel access to user memory.
 *
 * A user virtual address is only meaningful inside its own process pagemap,
 * and it may be unmapped, kernel-owned or read-only.  Never dereference one
 * directly from kernel code: translate it here first, which validates the
 * address and hands back an HHDM pointer to the same physical page.
 */

/* First non-canonical address above the user half. */
#define USER_VA_MAX 0x0000800000000000ULL

/* True if [uva, uva+n) lies wholly inside the user half and does not wrap. */
int user_range_ok(uint64_t uva, size_t n);

/*
 * Translate one user address to a kernel pointer, or NULL if it is not
 * mapped, not user-accessible, or (when for_write) not writable.  The
 * result is valid for the rest of the page only.
 */
void *user_kptr(process_t *proc, uint64_t uva, int for_write);

/* All three return 0 on success and -1 if any part of the range is bad. */
int copy_from_user(process_t *proc, void *kdst, uint64_t usrc, size_t n);
int copy_to_user(process_t *proc, uint64_t udst, const void *ksrc, size_t n);

/*
 * Copy a NUL-terminated string in.  kdst is always left NUL-terminated.
 * Returns -1 if the string is unreadable or longer than max - 1 bytes.
 */
int copy_user_str(process_t *proc, char *kdst, uint64_t usrc, size_t max);
