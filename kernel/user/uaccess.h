#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../proc/process.h"

void *user_kptr(process_t *proc, uint64_t uva);

int copy_from_user(process_t *proc, void *kdst, uint64_t usrc, size_t n);
int copy_to_user(process_t *proc, uint64_t udst, const void *ksrc, size_t n);
int copy_user_str(process_t *proc, char *kdst, uint64_t usrc, size_t max);
