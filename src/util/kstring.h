/**
** @file	kstring.h
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	Non-null-terminated string library declerations
*/

#ifndef __K_STRING_H__
#define __K_STRING_H__

#define SP_KERNEL_SRC
#include "common.h"

typedef struct kstr
{
    char *str;
    uint32_t len;
} kstr_t;

/**
 * Macro to easily create a kstring_t
 */
#define KSTR_CREATE(str, len) { (str), (len) }
/**
 * Macro to easily create a kstring_t without passing a length
 */
#define KSTR_LIT_CREATE(str) { (str), __strlen(str) }

/**
 * Test whether a kstring_t is empty
 */
#define KSTR_EMPTY { NULL, 0 }
/**
 * Get the length of a kstring_t
 */
#define KSTR_STRLEN(str) ((str)->len)
/**
 * Test whether two kstring_t's are equal
 */
#define KSTR_IS_EQUAL(left, right) (kstr_strcmp((left), (right)) == 0)

/**
 * @brief Compare two kstring_t's
 *
 * This behaves identically to the normal strcmp except it
 * operates on kstring_t's.
 *
 * @param left the first string to compare
 * @param right the second string to compare
 *
 * @return int negative value if the left < right,
 *         positive if left > right,
 *         and 0 if they're equal
 */
int kstr_strcmp(kstr_t *left, kstr_t *right);

/**
 * @brief The context used to link subsequent calls of kstr_strtok
 */
typedef struct kstr_strtok_context
{
    char *last_end;
    // TODO(Adin): Finish me
} kstr_strtok_context_t;

/**
 * @brief Tokenize a kstring_t
 *
 * Sequential calls (with the same context) return adjacent tokens in
 * the string, seperated by delim.
 *
 * @param str the string to tokenize
 * @param delim the delimiter character seperating adjacent tokens
 * @param context the private context of a single string being tokenized
 *
 * @return kstr_t the nth token of str, delimited by delim
 */
kstr_t kstr_strtok(kstr_t *str, char delim, kstr_strtok_context_t *context);

/**
 *
 * 32bit implementation of FNV-1 hash (parameters were found in second link)
 * https://en.wikipedia.org/w/index.php?title=Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 * http://isthe.com/chongo/tech/comp/fnv
 *
 * FHV-1 was chosen over FHV-1a for its lower collision rate (at the cost of slightly longer
 * average runtime on the order of ns) when hashing lowercase strings according to
 * https://softwareengineering.stackexchange.com/a/145633
 *
 * NOTE(Adin): Not sure where to stick this, but here is a link to some alternative, interesting
 * qnd hashes
 * http://www.cse.yorku.ca/~oz/hash.html
 *
*/
uint32_t kstr_hash(kstr_t *str);

#endif // #ifndef __K_STRING_H__