
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "utility.h"

#define MAXLINESIZE 256

static void parsePairs(connectionProfile *profile, const char *key, const char *value) {
    if (strncmp(key, "hostname", 8) == 0) {
        profile->hostname = strdup(value);
    } else if (strncmp(key, "portnumber", 10) == 0) {
        profile->portnumber = strtoul(value, NULL, 10);
    } else if (strncmp(key, "gamekindname", 12) == 0) {
        profile->gamekindname = strdup(value);
    }
}

connectionProfile *readConfig(connectionProfile *profile, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (NULL == file) {
        perror("fopen");
        return NULL;
    }

    char line[MAXLINESIZE];
    while (fgets(line, sizeof(line), file) != NULL) {
        int n_tokens  = 0;
        char **tokens = splitBy(line, " =\n");

        if (NULL == tokens) {
            perror("splitBy");
            return NULL;
        }

        char **p = tokens;
        while (*p != NULL) {
            n_tokens++;
            p++;
        }

        if (n_tokens >= 1) parsePairs(profile, tokens[0], tokens[1]);

        free(tokens);
    }

    fclose(file);
    return profile;
}
