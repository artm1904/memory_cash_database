cmake_minimum_required(VERSION 3.10)


# Этот файл определяет два "таргета" (цели сборки):
# 1. binary_tree - статическая библиотека с вашей структурой данных.
# 2. database_server - исполняемый файл сервера.

# Библиотека, содержащая только логику дерева.
add_library(binary_tree
    source/tree.cpp
)

# Делаем заголовочные файлы библиотеки доступными для других таргетов.
target_include_directories(binary_tree
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(database_server
    source/server.cpp
)

target_include_directories(database_server
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(database_server PRIVATE binary_tree)


# Применяем флаги компиляции, которые мы определили в родительском CMakeLists.txt
target_compile_options(database_server  PRIVATE ${PROJECT_COMPILE_OPTIONS})
target_compile_options(binary_tree  PRIVATE ${PROJECT_COMPILE_OPTIONS})


