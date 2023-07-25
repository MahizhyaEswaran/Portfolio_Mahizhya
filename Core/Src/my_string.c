/*
 * memmem() string compare function Implementation
 * By: Anistus H
 * 2023/05/02
 */

#include "my_string.h"
#include <string.h>

void *my_memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen)
{
    if (needlelen > haystacklen)
        return NULL;

    const char *haystack_ptr = (const char *) haystack;
    const char *needle_ptr = (const char *) needle;
    const char *end_ptr = haystack_ptr + haystacklen - needlelen + 1;

    while (haystack_ptr < end_ptr) {
        if (*haystack_ptr == *needle_ptr && memcmp(haystack_ptr, needle_ptr, needlelen) == 0)
            return (void *) haystack_ptr;
        haystack_ptr++;
    }

    return NULL;
}
