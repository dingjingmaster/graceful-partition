//
// Created by dingjing on 4/21/22.
//

#ifndef GRACEFUL_PARTITION_ALL_IO_H
#define GRACEFUL_PARTITION_ALL_IO_H

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sendfile.h>

#include "global.h"

ssize_t read_all(int fd, char *buf, size_t count);
int write_all(int fd, const void *buf, size_t count);
ssize_t sendfile_all(int out, int in, off_t *off, size_t count);
int fwrite_all(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif //GRACEFUL_PARTITION_ALL_IO_H
