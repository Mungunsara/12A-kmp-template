#include "search.h"
#include <stdlib.h>
#include <stdint.h>

/* KMP-ийн DFA хүснэгтийг байгуулах */
void kmp_build_dfa(const unsigned char *pattern, size_t m,
                   size_t *dfa, size_t *restart)
{
    *restart = 0;

    for (size_t c = 0; c < KMP_ALPHABET; ++c) {
        for (size_t j = 0; j < m; ++j) {
            dfa[c * m + j] = 0;
        }
    }

    if (m == 0)
        return;

    dfa[(size_t)pattern[0] * m] = 1;

    size_t x = 0;

    for (size_t j = 1; j < m; ++j) {
        for (size_t c = 0; c < KMP_ALPHABET; ++c) {
            dfa[c * m + j] = dfa[c * m + x];
        }

        dfa[(size_t)pattern[j] * m + j] = j + 1;

        x = dfa[(size_t)pattern[j] * m + x];
    }

    *restart = x;
}

/* DFA ашиглан бүх олдцыг хайх */
int search_all(const unsigned char *text, size_t n,
               const unsigned char *pattern, size_t m,
               match_fn emit, void *context)
{
    if (m == 0 || pattern == NULL || emit == NULL ||
        (n > 0 && text == NULL)) {
        return SEARCH_BAD_ARGUMENT;
    }

    if (m > SIZE_MAX / KMP_ALPHABET) {
        return SEARCH_BAD_ARGUMENT;
    }

    size_t entries = KMP_ALPHABET * m;

    if (entries > SIZE_MAX / sizeof(size_t)) {
        return SEARCH_BAD_ARGUMENT;
    }

    if (n < m)
        return SEARCH_OK;

    size_t *dfa = malloc(entries * sizeof(size_t));

    if (dfa == NULL)
        return SEARCH_NO_MEMORY;

    size_t restart = 0;

    kmp_build_dfa(pattern, m, dfa, &restart);

    size_t state = 0;

    for (size_t i = 0; i < n; ++i) {
        unsigned char c = text[i];

        state = dfa[(size_t)c * m + state];

        if (state == m) {
            size_t position = i + 1 - m;

            if (emit(position, context) != 0) {
                free(dfa);
                return SEARCH_OUTPUT_ERROR;
            }

            state = restart;
        }
    }

    free(dfa);

    return SEARCH_OK;
}
