#pragma once

#include <stddef.h>
#include <stdint.h>

size_t      str_insert(char *value, uint8_t isConstant);
const char *str_get(uint32_t key);

void   str_ref_incr(uint32_t key);
void   str_ref_decr(uint32_t key);
size_t str_len(uint32_t key);

void str_free();
// Release the memory associated with
// a String
size_t str_release(void *s);
