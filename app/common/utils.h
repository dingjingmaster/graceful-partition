//
// Created by dingjing on 4/22/22.
//

#ifndef GRACEFUL_PARTITION_UTILS_H
#define GRACEFUL_PARTITION_UTILS_H
#include "utils.h"
#include <time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

int xusleep(unsigned int usec);
struct dirent *xreaddir(DIR *dp);

#endif //GRACEFUL_PARTITION_UTILS_H
