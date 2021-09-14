/*
 * @(#)main.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define NUM_LANDMARKS  (3)
#define PERMISSION_NUM (0777)

void
rek_mkdir(char* path)
{
    char* sep = strrchr(path, '/');
    if (sep != NULL) {
        *sep = 0;
        rek_mkdir(path);
        *sep = '/';
        if (mkdir(path, PERMISSION_NUM) && errno != EEXIST) {
            printf("error while trying to create '%s'\n%m\n", path);
        }
    }
}

int
main(void)
{
    return 0;
}
