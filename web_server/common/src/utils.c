#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

char* utils_read_file(const char* path, size_t* size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    struct stat st;
    if (stat(path, &st) != 0) {
        fclose(f);
        return NULL;
    }

    *size = st.st_size;
    char* buf = malloc(*size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t read_bytes = fread(buf, 1, *size, f);
    if (read_bytes != *size) {
        free(buf);
        fclose(f);
        return NULL;
    }

    buf[*size] = '\0';
    fclose(f);
    return buf;
}