//
// Created by dingjing on 4/21/22.
//

#include "../app/common/path.h"

#include <getopt.h>
#include <inttypes.h>

static void __attribute__((__noreturn__)) usage(void)
{
    fprintf(stdout, "[options] <dir> <command>\n\n");
    fputs(" -p, --prefix <dir>      redirect hardcoded paths to <dir>\n", stdout);

    fputs(" Commands:\n", stdout);
    fputs(" read-u64 <file>            read uint64_t from file\n", stdout);
    fputs(" read-s64 <file>            read  int64_t from file\n", stdout);
    fputs(" read-u32 <file>            read uint32_t from file\n", stdout);
    fputs(" read-s32 <file>            read  int32_t from file\n", stdout);
    fputs(" read-string <file>         read string  from file\n", stdout);
    fputs(" read-majmin <file>         read devno from file\n", stdout);
    fputs(" read-link <file>           read symlink\n", stdout);
    fputs(" write-string <file> <str>  write string from file\n", stdout);
    fputs(" write-u64 <file> <str>     write uint64_t from file\n", stdout);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int c;
    const char *prefix = NULL, *dir, *file, *command;
    PathCxt *pc = NULL;

    static const struct option longopts[] = {
            { "prefix",	1, NULL, 'p' },
            { "help",       0, NULL, 'h' },
            { NULL, 0, NULL, 0 },
    };

    while((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
        switch(c) {
            case 'p':
                prefix = optarg;
                break;
            case 'h':
                usage();
                break;
            default:
                puts("try --help");
        }
    }

    if (optind == argc)
        puts("<dir> not defined");
    dir = argv[optind++];

    pc = path_new_path("%s", dir);
    if (!pc)
        puts("failed to initialize path context");
    if (prefix)
        path_set_prefix(pc, prefix);

    if (optind == argc)
        puts("<command> not defined");
    command = argv[optind++];

    puts (command);
    if (strcmp(command, "read-u32") == 0) {
        uint32_t res;

        if (optind == argc)
            puts("<file> not defined");
        file = argv[optind++];

        if (path_read_u32(pc, &res, file) != 0)
            puts("read u64 failed");
        printf("read:  %s: %u\n", file, res);

        if (path_readf_u32(pc, &res, "%s", file) != 0)
            puts("readf u64 failed");
        printf("readf: %s: %u\n", file, res);

    } else if (strcmp(command, "read-s32") == 0) {
        int32_t res;

        if (optind == argc)
            puts("<file> not defined");
        file = argv[optind++];

        if (path_read_s32(pc, &res, file) != 0)
            puts("read u64 failed");
        printf("read:  %s: %d\n", file, res);

        if (path_readf_s32(pc, &res, "%s", file) != 0)
            puts("readf u64 failed");
        printf("readf: %s: %d\n", file, res);

    } else if (strcmp(command, "read-u64") == 0) {
        uint64_t res;

        if (optind == argc)
            puts("<file> not defined");
        file = argv[optind++];

        if (path_read_u64(pc, &res, file) != 0)
            puts("read u64 failed");
        printf("read:  %s: %lu\n", file, res);

        if (path_readf_u64(pc, &res, "%s", file) != 0)
            puts("readf u64 failed");
        printf("readf: %s: %lu\n", file, res);

    } else if (strcmp(command, "read-s64") == 0) {
        int64_t res;

        if (optind == argc)
            puts("<file> not defined");
        file = argv[optind++];

        if (path_read_s64(pc, &res, file) != 0)
            puts("read u64 failed");
        printf("read:  %s: %ld\n", file, res);

        if (path_readf_s64(pc, &res, "%s", file) != 0)
            puts("readf u64 failed");
        printf("readf: %s: %ld\n", file, res);

    } else if (strcmp(command, "read-majmin") == 0) {
        dev_t res;

        if (optind == argc)
            puts("<file> not defined");
        file = argv[optind++];

        if (path_read_majmin(pc, &res, file) != 0)
            puts("read maj:min failed");
        printf("read:  %s: %d\n", file, (int) res);

        if (path_readf_majmin(pc, &res, "%s", file) != 0)
            puts("readf maj:min failed");
        printf("readf: %s: %d\n", file, (int) res);

    } else if (strcmp(command, "read-string") == 0) {
        char *res;

        if (optind == argc)
            puts("<file> not defined");
        file = argv[optind++];

        if (path_read_string(pc, &res, file) <= 0)
            puts("read string failed");
        printf("read:  %s: %s\n", file, res);

        if (path_readf_string(pc, &res, "%s", file) <= 0)
            puts("readf string failed");
        printf("readf: %s: %s\n", file, res);

    } else if (strcmp(command, "read-link") == 0) {
        char res[PATH_MAX];

        if (optind == argc)
            puts("<file> not defined");
        file = argv[optind++];

        if (path_readlink(pc, res, sizeof(res), file) < 0)
            puts("read symlink failed");
        printf("read:  %s: %s\n", file, res);

        if (path_readlinkf(pc, res, sizeof(res), "%s", file) < 0)
            puts("readf symlink failed");
        printf("readf: %s: %s\n", file, res);

    } else if (strcmp(command, "write-string") == 0) {
        char *str;
        puts ("kkk");

        if (optind + 1 == argc)
            puts("<file> <string> not defined");
        file = argv[optind++];
        str = argv[optind++];

        if (path_write_string(pc, str, file) != 0)
            fputs("write string failed", stdout);
        if (path_writef_string(pc, str, "%s", file) != 0)
            fputs("writef string failed", stdout);

    } else if (strcmp(command, "write-u64") == 0) {
        uint64_t num;

        if (optind + 1 == argc)
            puts("<file> <num> not defined");
        file = argv[optind++];
        num = strtoumax(argv[optind++], NULL, 0);

        if (path_write_u64(pc, num, file) != 0)
            puts( "write u64 failed");
        if (path_writef_u64(pc, num, "%s", file) != 0)
            puts("writef u64 failed");
    }

    path_unref_path(pc);
    return EXIT_SUCCESS;
}

