FILE(GLOB GRACEFUL_PARTITION_COMMON
        ${CMAKE_SOURCE_DIR}/app/common/global.h
        ${CMAKE_SOURCE_DIR}/app/common/path.h ${CMAKE_SOURCE_DIR}/app/common/path.c
        ${CMAKE_SOURCE_DIR}/app/common/utils.h ${CMAKE_SOURCE_DIR}/app/common/utils.c
        ${CMAKE_SOURCE_DIR}/app/common/bitops.h ${CMAKE_SOURCE_DIR}/app/common/bitops.c
        ${CMAKE_SOURCE_DIR}/app/common/blkdev.h ${CMAKE_SOURCE_DIR}/app/common/blkdev.c
        ${CMAKE_SOURCE_DIR}/app/common/all-io.h ${CMAKE_SOURCE_DIR}/app/common/all-io.c
        ${CMAKE_SOURCE_DIR}/app/common/file-utils.h ${CMAKE_SOURCE_DIR}/app/common/file-utils.c
        ${CMAKE_SOURCE_DIR}/app/common/linux-version.h ${CMAKE_SOURCE_DIR}/app/common/linux-version.c
        )