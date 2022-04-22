//
// Created by dingjing on 4/21/22.
//

#include <grp.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <assert.h>


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include <fcntl.h>

#ifdef O_CLOEXEC
#define UL_CLOEXECSTR	"e"
#else
#define UL_CLOEXECSTR	""
#endif

#define stringify_value(s) stringify(s)
#define stringify(s) #s