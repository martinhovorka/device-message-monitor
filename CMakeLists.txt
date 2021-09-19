cmake_minimum_required(VERSION 3.5)

set(
    PROJECTNAME
    device-monitor
)

set(
    CMAKE_INSTALL_MESSAGE
    NEVER
)

project(${PROJECTNAME})

# security flags for g++ compiler
set(
    SECURITY_FLAGS
    " -D_FORTIFY_SOURCE=2 -Wconversion -Wsign-conversion -Wformat-security -fPIC -fPIE -fpic -fpie -fstack-protector-all -fstack-clash-protection -Wl,-z,noexecstack -Wl,-z,relro,-z,now"
)

# set common c++ build flags
set(
    CMAKE_CXX_FLAGS
    "-Wall -Wextra -Wpedantic -std=c++11 ${SECURITY_FLAGS}"
)

# set flags for debug build
set(
    CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS} -O0 -g3"
)

# set flags for minimum size release build
set(
    CMAKE_CXX_FLAGS_MINSIZEREL
    "${CMAKE_CXX_FLAGS} -Os -g0 -s -DNDEBUG"
)

# set flags for relase build with debug info
set(
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "${CMAKE_CXX_FLAGS} -O2 -g"
)

# set flags for relase build with full optimization
set(
    CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS} -O3 -g0 -s -DNDEBUG"
)

# set common configuration prefix
set(
    DIR_ETC
    etc
)

# get path to templates
get_filename_component(
    DIR_ETC_CMAKE_PREFIX
    ./${DIR_ETC}/cmake
    ABSOLUTE
)

# set path to binary template
set(
    TEMPLATE_BINARY
    ${DIR_ETC_CMAKE_PREFIX}/BinaryTemplate.cmake
)

# set path to static library template
set(
    TEMPLATE_STATIC_LIBRARY
    ${DIR_ETC_CMAKE_PREFIX}/StaticLibraryTemplate.cmake
)

# set installation prefix
get_filename_component(
    INSTALL_PREFIX
    ./bin
    ABSOLUTE
)

# add project directory
add_subdirectory(src)

