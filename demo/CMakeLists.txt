project (demo)

aux_source_directory(. demo)
#include_directories(${GLIB_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS})

foreach(src ${demo})
    get_filename_component(mainName ${src} NAME_WE)
    add_executable(${mainName} ${src})
    target_link_libraries(${mainName} "${PARTED_LIBRARIES}")
endforeach(src)