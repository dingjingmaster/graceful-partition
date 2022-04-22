//
// Created by dingjing on 4/22/22.
//

#include "utils.h"

#include <string.h>

int xusleep(unsigned int usec)
{
    return usleep(usec);
}
struct dirent *xreaddir(DIR *dp)
{
    struct dirent *d;

    while ((d = readdir(dp))) {
        if (!strcmp(d->d_name, ".") ||
            !strcmp(d->d_name, ".."))
            continue;
        break;
    }
    return d;
}