#include <stdio.h>

typedef struct Cursor {
    FILE* file;
    fpos_t offset;
} cursor_t;
