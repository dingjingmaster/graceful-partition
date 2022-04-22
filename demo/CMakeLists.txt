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
