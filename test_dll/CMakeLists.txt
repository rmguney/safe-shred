cmake_minimum_required(VERSION 3.24)
project(sash_test_dll LANGUAGES C)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(sash_test SHARED
    ../core/shred.c ../core/strategy.c ../core/util.c 
    ../core/shred.h ../core/strategy.h ../core/util.h)
target_include_directories(sash_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../core)
target_link_libraries(sash_test PRIVATE advapi32 crypt32)
target_compile_definitions(sash_test PRIVATE SASH_API=__declspec\(dllexport\))
