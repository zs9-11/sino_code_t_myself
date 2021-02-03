cmake_minimum_required (VERSION 2.8.0)

set(LIBNAME ntripclient)

#include_directories($ENV{HOME}/vendor/sa415m/oelinuxRtk/util)
include_directories(${PROJECT_SOURCE_DIR}/util-N)
#link_directories($ENV{HOME}/vendor/oelinuxRtk/util/lib/qcomARMv7)
link_directories(${LIBRARY_OUTPUT_PATH})

aux_source_directory(. SRC_DIR)

add_library(${LIBNAME} SHARED ${SRC_DIR})
target_link_libraries(${LIBNAME}  "libutil-N${CMAKE_STATIC_LIBRARY_SUFFIX_C}")


