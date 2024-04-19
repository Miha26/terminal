#include <string.h>
#include <stdlib.h>
#include "dirname.h"

char *my_dirname(const char *path) {
    char *result = NULL;
    char *copy = strdup(path);

    if (copy != NULL) {
        char *last_slash = strrchr(copy, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
            result = strdup(copy);
        }
        free(copy);
    }

    return result;
}