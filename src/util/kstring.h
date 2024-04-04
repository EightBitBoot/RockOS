#ifndef __K_STRING_H__
#define __K_STRING_H__

#define SP_KERNEL_SRC
#include "common.h"

typedef struct kstr
{
    char *str;
    uint32_t len;
} kstr_t;

#define KSTR_CREATE(str, len) { (str), (len) }
#define KSTR_LIT_CREATE(str) { (str), __strlen(str) }

#define KSTR_EMPTY { NULL, 0 }
#define KSTR_STRLEN(str) ((str)->len)

#define KSTR_IS_EQUAL(left, right) (kstr_strcmp((left), (right)) == 0)

int kstr_strcmp(kstr_t *left, kstr_t *right);

typedef struct kstr_strtok_context
{
    char *last_end;
    // TODO(Adin): Finish me
} kstr_strtok_context_t;

/**
 * Yes I know that delim is traditionally a string of delimiters but
 * this is currently, mostly intended for pathname splitting and I
 * can't be bothered to check multiple delimter chars per input char.
 *
 * If other people want to use this and adapt it, they are more than
 * welcome to be my guest.
 *
 * - Adin W-T
 *
 * PS. Requiring a context parameter prevents the stupid clib variation
 * where a global variable is used.
*/
kstr_t kstr_strtok(kstr_t *str, char delim, kstr_strtok_context_t *context);

uint32_t kstr_hash(kstr_t *str);

#endif // #ifndef __K_STRING_H__