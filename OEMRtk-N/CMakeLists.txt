cmake_minimum_required (VERSION 2.8.0)

set(LIBNAME PreRtk-N)

include_directories(${PROJECT_SOURCE_DIR}/util-N)

if(EC100Y_PLATFORM)
include_directories(${GNSS_SDK_INCLUDE})
endif()

link_directories(${RTK_SDK_LIBPATH})
link_directories(${LIBRARY_OUTPUT_PATH})
aux_source_directory(. SRC_DIR)

add_library(${LIBNAME} STATIC ${SRC_DIR})
target_link_libraries(${LIBNAME} util-N ${STDLIB}) 
#target_link_libraries(${LIBNAME} util "${RTK_SDK_LIBPATH}/liboemRtk-linux-gnueabi.a") 

