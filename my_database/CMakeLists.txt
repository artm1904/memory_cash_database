cmake_minimum_required(VERSION 3.10)


set(EXE_NAME executable)


add_executable(${EXE_NAME}
    source/tree.cpp
)


target_include_directories(${EXE_NAME}
    PRIVATE
    include/
)


# Применяем флаги компиляции, которые мы определили в родительском CMakeLists.txt
target_compile_options(${EXE_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS})


