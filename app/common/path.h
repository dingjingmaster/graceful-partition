//
// Created by dingjing on 4/21/22.
//

#ifndef GRACEFUL_PARTITION_PATH_H
#define GRACEFUL_PARTITION_PATH_H

#include <dirent.h>
#include "global.h"

typedef struct _PathCxt PathCxt;

struct _PathCxt
{
    int	    dirFd;
    char   *dirPath;
    int	    refcount;

    char   *prefix;
    char    pathBuffer[PATH_MAX];

    void    *dialect;
    void   (*free_dialect) (PathCxt*);
    int	   (*redirect_on_enoent) (PathCxt*, const char*, int*);
};

void path_ref_path (PathCxt* pc);
void path_unref_path (PathCxt* pc);
PathCxt* path_new_path (const char *dir, ...);

const char *path_get_prefix (PathCxt* pc);
int path_set_prefix (PathCxt* pc, const char *prefix);

const char *path_get_dir (PathCxt* pc);
int path_set_dir (PathCxt* pc, const char *dir);

void *path_get_dialect (PathCxt* pc);
int path_set_dialect (PathCxt* pc, void *data, void free_data(PathCxt*));

int path_get_dirfd (PathCxt* pc);
void path_close_dirfd (PathCxt* pc);
int path_isopen_dirfd (PathCxt* pc);
int path_set_enoent_redirect (PathCxt* pc, int (*func)(PathCxt* , const char *, int *));

char *path_get_abspath (PathCxt* pc, char *buf, size_t bufsz, const char *path, ...) __attribute__ ((__format__ (__printf__, 4, 5)));

int path_access (PathCxt* pc, int mode, const char *path);
int path_accessf (PathCxt* pc, int mode, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_open (PathCxt* pc, int flags, const char *path);
int path_openf (PathCxt* pc, int flags, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));
int path_vopenf (PathCxt* pc, int flags, const char *path, va_list ap);

FILE* path_fopen (PathCxt* pc, const char *mode, const char *path);
FILE* path_fopenf (PathCxt* pc, const char *mode, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));
FILE* path_vfopenf (PathCxt* pc, const char *mode, const char *path, va_list ap);

DIR* path_opendir (PathCxt* pc, const char *path);
DIR* path_vopendirf (PathCxt* pc, const char *path, va_list ap);
DIR* path_opendirf (PathCxt* pc, const char *path, ...) __attribute__ ((__format__ (__printf__, 2, 3)));

ssize_t path_readlink (PathCxt* pc, char *buf, size_t bufsiz, const char *path);
ssize_t path_readlinkf (PathCxt* pc, char *buf, size_t bufsiz, const char *path, ...) __attribute__ ((__format__ (__printf__, 4, 5)));

int path_read (PathCxt* pc, char *buf, size_t len, const char *path);
int path_vreadf (PathCxt* pc, char *buf, size_t len, const char *path, va_list ap);
int path_readf (PathCxt* pc, char *buf, size_t len, const char *path, ...) __attribute__ ((__format__ (__printf__, 4, 5)));

int path_read_string (PathCxt* pc, char **str, const char *path);
int path_readf_string (PathCxt* pc, char **str, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_read_buffer (PathCxt* pc, char *buf, size_t bufsz, const char *path);
int path_readf_buffer (PathCxt* pc, char *buf, size_t bufsz, const char *path, ...) __attribute__ ((__format__ (__printf__, 4, 5)));

int path_scanf (PathCxt* pc, const char *path, const char *fmt, ...);
int path_scanff (PathCxt* pc, const char *path, va_list ap, const char *fmt, ...) __attribute__ ((__format__ (__scanf__, 4, 5)));

int path_read_majmin (PathCxt* pc, dev_t *res, const char *path);
int path_readf_majmin (PathCxt* pc, dev_t *res, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_read_u32 (PathCxt* pc, uint32_t *res, const char *path);
int path_readf_u32 (PathCxt* pc, uint32_t *res, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_read_s32 (PathCxt* pc, int32_t *res, const char *path);
int path_readf_s32 (PathCxt* pc, int32_t *res, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_read_u64 (PathCxt* pc, uint64_t *res, const char *path);
int path_readf_u64 (PathCxt* pc, uint64_t *res, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_read_s64 (PathCxt* pc, int64_t *res, const char *path);
int path_readf_s64 (PathCxt* pc, int64_t *res, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_write_string (PathCxt* pc, const char *str, const char *path);
int path_writef_string (PathCxt* pc, const char *str, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_write_s64 (PathCxt* pc, int64_t num, const char *path);
int path_write_u64 (PathCxt* pc, uint64_t num, const char *path);
int path_writef_u64 (PathCxt* pc, uint64_t num, const char *path, ...) __attribute__ ((__format__ (__printf__, 3, 4)));

int path_count_dirents (PathCxt* pc, const char *path);
int path_countf_dirents (PathCxt* pc, const char *path, ...) __attribute__ ((__format__ (__printf__, 2, 3)));

FILE* path_prefix_fopen (const char *prefix, const char *path, const char *mode);

#endif // GRACEFUL_PARTITION_PATH_H
