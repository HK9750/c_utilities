#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

char* utils_read_file(const char* path, size_t* size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    struct stat st;
    stat(path, &st);
    *size = st.st_size;
    char* buf = malloc(*size + 1);
    fread(buf, 1, *size, f);
    buf[*size] = '\0';
    fclose(f);
    return buf;
}