#pragma once

#include <stddef.h>
#include <stdint.h>

uint32_t    str_insert(const char *value);
const char *str_get(uint32_t key);

void   str_ref_incr(uint32_t key);
void   str_ref_decr(uint32_t key);
size_t str_len(uint32_t key);

void str_free();
