project (demo)

pkg_check_modules(PARTED REQUIRED libparted)

#aux_source_directory(. demo)
#
#foreach(src ${demo})
#    get_filename_component(mainName ${src} NAME_WE)
#    add_executable(${mainName} ${src})
#    target_link_libraries(${mainName} "${PARTED_LIBRARIES}")
#endforeach(src)


add_executable(demo-list-device demo-list-device.c)
target_link_libraries(demo-list-device "${PARTED_LIBRARIES}")

add_executable(demo-path demo-path.c ../app/common/path.c ../app/common/all-io.c ../app/common/utils.c)

add_executable(demo-partition demo-partition.c ../app/partitions/partitions.c ../app/partitions/partitions-mbr.c)
add_executable(demo-bitops demo-bitops.c ../app/common/bitops.c)
add_executable(demo-blkdev demo-blkdev.c ../app/common/blkdev.c)
add_executable(demo-linux-version demo-linux-version.c ../app/common/linux-version.c)
add_executable(demo-file-utils demo-file-utils.c ../app/common/file-utils.c)
add_executable(demo-path-name demo-path-name.c ../app/common/path-name.c)