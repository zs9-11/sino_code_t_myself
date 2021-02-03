cmake_minimum_required (VERSION 2.8.0)

set(LIBNAME ntripclient)

aux_source_directory(. SRC_DIR)

include_directories(${PROJECT_SOURCE_DIR}/util-N)
#include_directories(../util ${GNSS_SDK_INCLUDE}/common/include/quectel/open ${GNSS_SDK_INCLUDE}/interface/network/sockets/inc ${GNSS_SDK_INCLUDE}/common/include ${GNSS_SDK_INCLUDE}/interface/fs/inc ${GNSS_SDK_INCLUDE}/interface/init/inc ${GNSS_SDK_INCLUDE}/interface/network/data_call/inc)
#include_directories(${GNSS_SDK_INCLUDE}/common/include/mbedtls ${GNSS_SDK_INCLUDE}/common/include/include/mbedtls ${GNSS_SDK_INCLUDE}/interface/os/inc ${GNSS_SDK_INCLUDE}/common/include/mbedtls/port/ASR/inc)
include_directories(${GNSS_SDK_INCLUDE})
link_directories(${GNSS_SDK_LIBRARY}/common/lib)

add_library(${LIBNAME} ${SRC_DIR})
target_link_libraries(${LIBNAME} ql_common_api.lib ${STDLIB})


