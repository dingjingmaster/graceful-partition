//
// Created by dingjing on 4/21/22.
//

#include "path.h"
#include "all-io.h"
#include "utils.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <sys/sysmacros.h>

static int mode2flags (const char *mode);
static const char *get_absdir (PathCxt *pc);
static int dup_fd_cloexec(int oldfd, int lowfd);
static inline void xstrncpy(char *dest, const char *src, size_t n);
static const char* ul_path_mkpath (PathCxt *pc, const char *path, va_list ap);

void path_ref_path (PathCxt* pc)
{
    if (pc) {
        ++pc->refcount;
    }
}

void path_unref_path (PathCxt* pc)
{
    if (!pc)
        return;

    pc->refcount--;

    if (pc->refcount <= 0) {
        if (pc->dialect)
            pc->free_dialect(pc);
        path_close_dirfd(pc);
        free(pc->dirPath);
        free(pc->prefix);
        free(pc);
    }
}

PathCxt* path_new_path (const char *dir, ...)
{
    PathCxt* pc = calloc(1, sizeof(*pc));

    if (!pc)
        return NULL;

    pc->refcount = 1;
    pc->dirFd = -1;

    if (dir) {
        int rc;
        va_list ap;

        va_start(ap, dir);
        rc = vasprintf(&pc->dirPath, dir, ap);
        va_end(ap);

        if (rc < 0 || !pc->dirPath)
            goto fail;
    }
    return pc;
fail:
    path_unref_path(pc);
    return NULL;
}

const char *path_get_prefix (PathCxt* pc)
{
    return pc ? pc->prefix : NULL;
}

int path_set_prefix (PathCxt* pc, const char *prefix)
{
    char *p = NULL;

    assert(pc->dirFd < 0);

    if (prefix) {
        p = strdup(prefix);
        if (!p)
            return -ENOMEM;
    }

    free(pc->prefix);
    pc->prefix = p;

    return 0;
}

const char *path_get_dir (PathCxt* pc)
{
    return pc ? pc->dirPath : NULL;
}

int path_set_dir (PathCxt* pc, const char *dir)
{
    char *p = NULL;

    if (dir) {
        p = strdup(dir);
        if (!p)
            return -ENOMEM;
    }

    if (pc->dirFd >= 0) {
        close(pc->dirFd);
        pc->dirFd = -1;
    }

    free(pc->dirPath);
    pc->dirPath = p;

    return 0;
}

void *path_get_dialect (PathCxt* pc)
{
    return pc ? pc->dialect : NULL;
}

int path_set_dialect (PathCxt* pc, void *data, void free_data(PathCxt*))
{
    pc->dialect = data;
    pc->free_dialect = free_data;

    return 0;
}

int path_get_dirfd (PathCxt* pc)
{
    assert(pc);
    assert(pc->dirPath);

    if (pc->dirFd < 0) {
        const char *path = get_absdir(pc);
        if (!path)
            return -errno;

        pc->dirFd = open(path, O_RDONLY | O_CLOEXEC);
    }

    return pc->dirFd;
}

void path_close_dirfd (PathCxt* pc)
{
    assert(pc);

    if (pc->dirFd >= 0) {
        close(pc->dirFd);
        pc->dirFd = -1;
    }
}

int path_isopen_dirfd (PathCxt* pc)
{
    return pc && pc->dirFd >= 0;
}

int path_set_enoent_redirect (PathCxt* pc, int (*func)(PathCxt*, const char*, int *))
{
    pc->redirect_on_enoent = func;
    return 0;
}

char* path_get_abspath (PathCxt* pc, char *buf, size_t bufsz, const char *path, ...)
{
    if (path) {
        int rc;
        va_list ap;
        const char *tail = NULL, *dirPath = pc->dirPath;

        va_start(ap, path);
        tail = ul_path_mkpath(pc, path, ap);
        va_end(ap);

        if (dirPath && *dirPath == '/')
            dirPath++;
        if (tail && *tail == '/')
            tail++;

        rc = snprintf(buf, bufsz, "%s/%s/%s",
                      pc->prefix ? pc->prefix : "",
                      dirPath ? dirPath : "",
                      tail ? tail : "");

        if ((size_t)rc >= bufsz) {
            errno = ENAMETOOLONG;
            return NULL;
        }
    } else {
        const char *tmp = get_absdir(pc);

        if (!tmp)
            return NULL;
        xstrncpy(buf, tmp, bufsz);
    }

    return buf;
}

int path_access (PathCxt* pc, int mode, const char *path)
{
    int dir, rc;

    dir = path_get_dirfd(pc);
    if (dir < 0)
        return dir;

    rc = faccessat(dir, path, mode, 0);

    if (rc && errno == ENOENT
        && pc->redirect_on_enoent
        && pc->redirect_on_enoent(pc, path, &dir) == 0)
        rc = faccessat(dir, path, mode, 0);
    return rc;
}

int path_accessf (PathCxt* pc, int mode, const char *path, ...)
{
    va_list ap;
    const char *p;

    va_start (ap, path);
    p = ul_path_mkpath (pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_access (pc, mode, p);
}

int path_open (PathCxt* pc, int flags, const char *path)
{
    int fd = 0;

    if (!pc) {
        fd = open(path, flags);
    } else {
        int fdx;
        int dir = path_get_dirfd(pc);
        if (dir < 0)
            return dir;

        fdx = fd = openat(dir, path, flags);

        if (fd < 0 && errno == ENOENT
            && pc->redirect_on_enoent
            && pc->redirect_on_enoent(pc, path, &dir) == 0)
            fd = openat(dir, path, flags);
    }
    return fd;
}

int path_vopenf (PathCxt* pc, int flags, const char *path, va_list ap)
{
    const char *p = ul_path_mkpath(pc, path, ap);

    return !p ? -errno : path_open(pc, flags, p);
}

int path_openf (PathCxt* pc, int flags, const char *path, ...)
{
    va_list ap;
    int rc;

    va_start(ap, path);
    rc = path_vopenf(pc, flags, path, ap);
    va_end(ap);

    return rc;
}

FILE* path_fopen (PathCxt* pc, const char *mode, const char *path)
{
    int flags = mode2flags(mode);
    int fd = path_open(pc, flags, path);

    if (fd < 0)
        return NULL;

    return fdopen(fd, mode);
}

FILE* path_vfopenf (PathCxt* pc, const char *mode, const char *path, va_list ap)
{
    const char *p = ul_path_mkpath(pc, path, ap);

    return !p ? NULL : path_fopen(pc, mode, p);
}

FILE* path_fopenf (PathCxt* pc, const char *mode, const char *path, ...)
{
    FILE *f;
    va_list ap;

    va_start(ap, path);
    f = path_vfopenf(pc, mode, path, ap);
    va_end(ap);

    return f;
}

DIR* path_opendir (PathCxt* pc, const char *path)
{
    DIR *dir;
    int fd = -1;

    if (path)
        fd = path_open(pc, O_RDONLY | O_CLOEXEC, path);
    else if (pc->dirPath) {
        int dirfd;

        dirfd = path_get_dirfd(pc);
        if (dirfd >= 0)
            fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
    }

    if (fd < 0)
        return NULL;

    dir = fdopendir(fd);
    if (!dir) {
        close(fd);
        return NULL;
    }
    if (!path)
        rewinddir(dir);

    return dir;
}

DIR* path_vopendirf (PathCxt* pc, const char *path, va_list ap)
{
    const char *p = ul_path_mkpath(pc, path, ap);

    return !p ? NULL : path_opendir(pc, p);
}

DIR* path_opendirf (PathCxt* pc, const char *path, ...)
{
    va_list ap;
    DIR *dir;

    va_start(ap, path);
    dir = path_vopendirf(pc, path, ap);
    va_end(ap);

    return dir;
}

ssize_t path_readlink (PathCxt* pc, char *buf, size_t bufsiz, const char *path)
{
    int dirfd;

    if (!path) {
        const char *p = get_absdir(pc);
        if (!p)
            return -errno;
        return readlink(p, buf, bufsiz);
    }

    dirfd = path_get_dirfd(pc);
    if (dirfd < 0)
        return dirfd;

    return readlinkat(dirfd, path, buf, bufsiz);
}

ssize_t path_readlinkf (PathCxt* pc, char *buf, size_t bufsiz, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_readlink(pc, buf, bufsiz, p);
}

int path_read (PathCxt* pc, char *buf, size_t len, const char *path)
{
    int rc;
    int fd, errsv;

    fd = path_open(pc, O_RDONLY|O_CLOEXEC, path);
    if (fd < 0)
        return -errno;

    rc = (int) read_all(fd, buf, len);

    errsv = errno;
    close(fd);
    errno = errsv;

    return rc;
}

int path_vreadf (PathCxt* pc, char *buf, size_t len, const char *path, va_list ap)
{
    const char *p = ul_path_mkpath(pc, path, ap);

    return !p ? -errno : path_read(pc, buf, len, p);
}

int path_readf (PathCxt* pc, char *buf, size_t len, const char *path, ...)
{
    va_list ap;
    int rc;

    va_start(ap, path);
    rc = path_vreadf(pc, buf, len, path, ap);
    va_end(ap);

    return rc;
}

int path_read_string (PathCxt* pc, char **str, const char *path)
{
    char buf[BUFSIZ];
    int rc;

    if (!str)
        return -EINVAL;

    *str = NULL;
    rc = path_read(pc, buf, sizeof(buf) - 1, path);
    if (rc < 0)
        return rc;

    /* Remove tailing newline (usual in sysfs) */
    if (rc > 0 && *(buf + rc - 1) == '\n')
        --rc;

    buf[rc] = '\0';
    *str = strdup(buf);
    if (!*str)
        rc = -ENOMEM;

    return rc;
}

int path_readf_string (PathCxt* pc, char **str, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_read_string(pc, str, p);
}

int path_read_buffer (PathCxt* pc, char *buf, size_t bufsz, const char *path)
{
    int rc = path_read(pc, buf, bufsz - 1, path);
    if (rc < 0)
        return rc;

    /* Remove tailing newline (usual in sysfs) */
    if (rc > 0 && *(buf + rc - 1) == '\n')
        --rc;

    buf[rc] = '\0';

    return rc;
}

int path_readf_buffer (PathCxt* pc, char *buf, size_t bufsz, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_read_buffer(pc, buf, bufsz, p);
}

int path_scanf (PathCxt* pc, const char *path, const char *fmt, ...)
{
    FILE *f;
    va_list fmt_ap;
    int rc;

    f = path_fopen(pc, "r" UL_CLOEXECSTR, path);
    if (!f)
        return -EINVAL;

    va_start(fmt_ap, fmt);
    rc = vfscanf(f, fmt, fmt_ap);
    va_end(fmt_ap);

    fclose(f);
    return rc;
}

int path_scanff (PathCxt* pc, const char *path, va_list ap, const char *fmt, ...)
{
    FILE *f;
    va_list fmt_ap;
    int rc;

    f = path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
    if (!f)
        return -EINVAL;

    va_start(fmt_ap, fmt);
    rc = vfscanf(f, fmt, fmt_ap);
    va_end(fmt_ap);

    fclose(f);

    return rc;
}

int path_read_majmin (PathCxt* pc, dev_t *res, const char *path)
{
    int rc, maj, min;

    rc = path_scanf(pc, path, "%d:%d", &maj, &min);
    if (rc != 2)
        return -1;
    if (res)
        *res = makedev(maj, min);
    return 0;
}

int path_readf_majmin (PathCxt* pc, dev_t *res, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_read_majmin(pc, res, p);
}

int path_read_u32 (PathCxt* pc, uint32_t *res, const char *path)
{
    int rc;
    unsigned int x;

    rc = path_scanf(pc, path, "%u", &x);
    if (rc != 1)
        return -1;
    if (res)
        *res = x;
    return 0;
}

int path_readf_u32 (PathCxt* pc, uint32_t *res, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_read_u32(pc, res, p);
}

int path_read_s32 (PathCxt* pc, int32_t *res, const char *path)
{
    int rc, x = 0;

    rc = path_scanf(pc, path, "%d", &x);
    if (rc != 1)
        return -1;
    if (res)
        *res = x;
    return 0;
}

int path_readf_s32 (PathCxt* pc, int32_t *res, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_read_s32(pc, res, p);
}

int path_read_u64 (PathCxt* pc, uint64_t *res, const char *path)
{
    uint64_t x = 0;
    int rc;

    rc = path_scanf(pc, path, "%"SCNu64, &x);
    if (rc != 1)
        return -1;
    if (res)
        *res = x;
    return 0;
}

int path_readf_u64 (PathCxt* pc, uint64_t *res, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_read_u64(pc, res, p);
}

int path_read_s64 (PathCxt* pc, int64_t *res, const char *path)
{
    int64_t x = 0;
    int rc;

    rc = path_scanf(pc, path, "%"SCNd64, &x);
    if (rc != 1)
        return -1;
    if (res)
        *res = x;

    return 0;
}

int path_readf_s64 (PathCxt* pc, int64_t *res, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_read_s64(pc, res, p);
}

int path_write_string (PathCxt* pc, const char *str, const char *path)
{
    int rc, errsv;
    int fd;

    puts ("0");
    fd = path_open(pc, O_WRONLY|O_CLOEXEC, path);
    if (fd < 0)
        return -errno;

    puts ("1");

    rc = write_all(fd, str, strlen(str));

    puts ("2");
    errsv = errno;
    close(fd);
    errno = errsv;
    return rc;
}

int path_writef_string (PathCxt* pc, const char *str, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_write_string(pc, str, p);
}

int path_write_s64 (PathCxt* pc, int64_t num, const char *path)
{
    char buf[sizeof(stringify_value(LLONG_MAX))];
    int rc, errsv;
    int fd, len;

    fd = path_open(pc, O_WRONLY|O_CLOEXEC, path);
    if (fd < 0)
        return -errno;

    len = snprintf(buf, sizeof(buf), "%" PRId64, num);
    if (len < 0 || (size_t) len >= sizeof(buf))
        rc = len < 0 ? -errno : -E2BIG;
    else
        rc = write_all(fd, buf, len);

    errsv = errno;
    close(fd);
    errno = errsv;
    return rc;
}

int path_write_u64 (PathCxt* pc, uint64_t num, const char *path)
{
    char buf[sizeof(stringify_value(ULLONG_MAX))];
    int rc, errsv;
    int fd, len;

    fd = path_open(pc, O_WRONLY|O_CLOEXEC, path);
    if (fd < 0)
        return -errno;

    len = snprintf(buf, sizeof(buf), "%" PRIu64, num);
    if (len < 0 || (size_t) len >= sizeof(buf))
        rc = len < 0 ? -errno : -E2BIG;
    else
        rc = write_all(fd, buf, len);

    errsv = errno;
    close(fd);
    errno = errsv;
    return rc;
}

int path_writef_u64 (PathCxt* pc, uint64_t num, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_write_u64(pc, num, p);
}

int path_count_dirents (PathCxt* pc, const char *path)
{
    DIR *dir;
    int r = 0;

    dir = path_opendir(pc, path);
    if (!dir)
        return 0;

    while (xreaddir(dir)) r++;

    closedir(dir);
    return r;
}

int path_countf_dirents (PathCxt* pc, const char *path, ...)
{
    const char *p;
    va_list ap;

    va_start(ap, path);
    p = ul_path_mkpath(pc, path, ap);
    va_end(ap);

    return !p ? -errno : path_count_dirents(pc, p);
}

FILE* path_prefix_fopen (const char *prefix, const char *path, const char *mode)
{
    char buf[PATH_MAX];

    if (!path)
        return NULL;
    if (!prefix)
        return fopen(path, mode);
    if (*path == '/')
        path++;

    snprintf(buf, sizeof(buf), "%s/%s", prefix, path);
    return fopen(buf, mode);
}


static const char* get_absdir (PathCxt *pc)
{
    int rc;
    const char* dirPath;

    if (!pc->prefix)
        return pc->dirPath;

    dirPath = pc->dirPath;
    if (!dirPath)
        return pc->prefix;
    if (*dirPath == '/')
        dirPath++;

    rc = snprintf(pc->pathBuffer, sizeof(pc->pathBuffer), "%s/%s", pc->prefix, dirPath);
    if (rc < 0)
        return NULL;
    if ((size_t)rc >= sizeof(pc->pathBuffer)) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    return pc->pathBuffer;
}

static const char* ul_path_mkpath (PathCxt *pc, const char *path, va_list ap)
{
    int rc;

    errno = 0;

    rc = vsnprintf(pc->pathBuffer, sizeof(pc->pathBuffer), path, ap);
    if (rc < 0) {
        if (!errno)
            errno = EINVAL;
        return NULL;
    }

    if ((size_t)rc >= sizeof(pc->pathBuffer)) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    return pc->pathBuffer;
}

static inline void xstrncpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n-1);
    dest[n-1] = 0;
}

static int mode2flags (const char *mode)
{
    int flags = 0;
    const char *p;

    for (p = mode; p && *p; p++) {
        if (*p == 'r' && *(p + 1) == '+')
            flags |= O_RDWR;
        else if (*p == 'r')
            flags |= O_RDONLY;

        else if (*p == 'w' && *(p + 1) == '+')
            flags |= O_RDWR | O_TRUNC;
        else if (*p == 'w')
            flags |= O_WRONLY | O_TRUNC;

        else if (*p == 'a' && *(p + 1) == '+')
            flags |= O_RDWR | O_APPEND;
        else if (*p == 'a')
            flags |= O_WRONLY | O_APPEND;
#ifdef O_CLOEXEC
        else if (*p == *UL_CLOEXECSTR)
            flags |= O_CLOEXEC;
#endif
    }

    return flags;
}

static int dup_fd_cloexec (int oldfd, int lowfd)
{
    int fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
    fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
    if (fd >= 0)
        return fd;
#endif

    fd = dup(oldfd);
    if (fd < 0)
        return fd;

    flags = fcntl(fd, F_GETFD);
    if (flags < 0)
        goto unwind;
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
        goto unwind;

    return fd;

unwind:
    errno_save = errno;
    close(fd);
    errno = errno_save;

    return -1;
}

#ifdef HAVE_CPU_SET_T
static int ul_path_cpuparse(struct path_cxt *pc, cpu_set_t **set, int maxcpus, int islist, const char *path, va_list ap)
{
	FILE *f;
	size_t setsize, len = maxcpus * 7;
	char buf[len];
	int rc;

	*set = NULL;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	if (!f)
		return -errno;

	rc = fgets(buf, len, f) == NULL ? -errno : 0;
	fclose(f);

	if (rc)
		return rc;

	len = strlen(buf);
	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	if (!*set)
		return -ENOMEM;

	if (islist) {
		if (cpulist_parse(buf, *set, setsize, 0)) {
			cpuset_free(*set);
			return -EINVAL;
		}
	} else {
		if (cpumask_parse(buf, *set, setsize)) {
			cpuset_free(*set);
			return -EINVAL;
		}
	}
	return 0;
}
int ul_path_readf_cpuset(struct path_cxt *pc, cpu_set_t **set, int maxcpus, const char *path, ...)
{
	va_list ap;
	int rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	return rc;
}

int ul_path_readf_cpulist(struct path_cxt *pc, cpu_set_t **set, int maxcpus, const char *path, ...)
{
	va_list ap;
	int rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	return rc;
}
#endif /* HAVE_CPU_SET_T */

