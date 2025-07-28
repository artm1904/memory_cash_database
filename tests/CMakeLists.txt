cmake_minimum_required(VERSION 3.10)
project(database_test VERSION 0.1.0)

enable_testing()

add_executable(${PROJECT_NAME}
    source/TreeTest.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${GOOGLETEST_SOURCE_DIR}/googletest/include
        
        

)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        binary_tree
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
