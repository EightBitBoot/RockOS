
#include "kstring.h"

#define KSTR_PTR_IDX(s, p) ((p) - (s)->str)
#define KSTR_PTR_INBNDS(s, p) (KSTR_PTR_IDX(s, p) < (s)->len)

int kstr_strcmp(kstr_t *left, kstr_t *right)
{
    if(left->len == 0 && right->len == 0) {
        return 0;
    }

    char *p_left = left->str;
    char *p_right = right->str;

    while(KSTR_PTR_INBNDS(left, p_left) &&
          KSTR_PTR_INBNDS(right, p_right) &&
          (*p_left == *p_right))
    {
        p_left++;
        p_right++;
    }

    /**
     * 4 Cases:
     *   1. end of both
     *   2. end of right
     *   3. end of left
     *   4. end of neither
    */

    bool_t left_at_end = !KSTR_PTR_INBNDS(left, p_left);
    bool_t right_at_end = !KSTR_PTR_INBNDS(right, p_right);

    if(left_at_end && right_at_end) {
        return *(p_left - 1) - *(p_right - 1);
    }

    if(right_at_end) {
        return *p_left;
    }

    if(left_at_end) {
        return -(*p_right);
    }

    return (*p_left) - (*p_right);
}

kstr_t kstr_strtok(kstr_t *str, char delim, kstr_strtok_context_t *context)
{
    char *curr_start;
    char *curr;

    if(!context->last_end) {
        curr_start = str->str;
    }
    else {
        curr_start = context->last_end + 1;
    }

    // Skip leading delims
    while(KSTR_PTR_INBNDS(str, curr_start) && *curr_start == delim) {
        curr_start++;
    }

    // The rest of the string is delimiters: return an empty string
    if(!KSTR_PTR_INBNDS(str, curr_start)) {
        kstr_t result = KSTR_EMPTY;
        return result;
    }

    // Find the current token
    curr = curr_start;
    while(KSTR_PTR_INBNDS(str, curr) && *curr != delim) {
        curr++;
    }

    kstr_t result = KSTR_CREATE(curr_start, curr - curr_start);
    context->last_end = curr;

    return result;
}

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
uint32_t kstr_hash(kstr_t *str)
{
    uint32_t hash = 2166136261;
    uint8_t curr_char;

    for(int i = 0; i < str->len; i++) {
        curr_char = str->str[i];
        hash = (hash * 16777619) ^ curr_char;
    }

    return hash;
}