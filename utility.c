#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"

char **splitBy(char *buff, const char *delims) {
    char **tokens = NULL;
    char *token   = strtok(buff, delims);
    int n_delims  = 0;

    // split string and append tokens to tokens
    while (NULL != token) {
        tokens = realloc(tokens, sizeof(char*) * (n_delims + 1));
        if (NULL == tokens) return NULL;
        tokens[n_delims++] = token;
        token = strtok(NULL, delims);
    }

    // realloc one extra element for the last NULL
    tokens = realloc(tokens, sizeof(char*) * (n_delims + 1));
    tokens[n_delims] = 0;

    return tokens;
}

int tokenLen(char **tokens) {
    int n_tokens = 0;
    char **token = tokens;
    while (*token != NULL) {
        n_tokens++;
        token++;
    }
    return n_tokens;
}
