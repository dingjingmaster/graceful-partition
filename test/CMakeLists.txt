project(test)

find_package(GTest REQUIRED)

add_executable(test_path test-path.cpp)
target_link_libraries(test_path ${GTEST_BOTH_LIBRARIES})

add_test(NAME test_path
        COMMAND test_path
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
#set_tests_properties (demo-path PROPERTIES PASS_ "is 4")
#add_executable(test-path test-path.c)
#target_link_libraries(test-path gtest gtest_main)

#include(GoogleTest)
#gtest_discover_tests(test-path)
