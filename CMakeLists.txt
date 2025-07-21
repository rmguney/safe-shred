cmake_minimum_required(VERSION 3.24)
project(sash LANGUAGES C)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(
    /W4                    # warning level 4
    /O2                    # maximum speed optimization
    /Oi                    # enable intrinsic functions
    /Ot                    # favor speed over size
    /Ox                    # maximum optimizations
    /GL                    # whole program optimization
    /favor:INTEL64         # optimize for x64
    /arch:AVX2             # use avx2 instructions if available
    /DWIN32_LEAN_AND_MEAN  # reduce Windows headers
    /DNDEBUG               # disable debug assertions
    /wd4100                # suppress unused parameter warnings
    /GS                    # buffer security check
    /sdl                   # additional security development lifecycle checks
)

add_link_options(
    /LTCG                  # link-time code generation
    /OPT:REF               # remove unreferenced code
    /OPT:ICF               # Iidentical COMDAT folding
    /INCREMENTAL:NO        # disable incremental linking for speed
    /DYNAMICBASE           # address space layout randomization (aslr)
    /NXCOMPAT              # data execution prevention (dep)
    /HIGHENTROPYVA         # 64-bit aslr
    /GUARD:CF              # control flow guard
)

# parallel compilation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# static library
add_library(sash_core_static STATIC
    core/shred.c core/strategy.c core/util.c core/shred.h core/strategy.h core/util.h)
target_include_directories(sash_core_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)
target_link_libraries(sash_core_static PRIVATE advapi32 crypt32)

add_executable(sash cli/sash_cli.c resources/sash_cli.rc)
target_link_libraries(sash PRIVATE sash_core_static advapi32 crypt32)

add_executable(sash_gui WIN32 gui/sash_gui.c resources/sash_gui.rc)
target_link_libraries(sash_gui PRIVATE sash_core_static comctl32 comdlg32 shell32 advapi32 crypt32)

install(TARGETS sash sash_gui RUNTIME DESTINATION .)
