set(CMAKE_SYSTEM_NAME Linux )

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(BUILD_SHARED_LIBS ON)
set(CMAKE_SHARED_LIBRARY_SUFFIX_C "-linux-gnueabi.so")
set(CMAKE_SHARED_LIBRARY_SUFFIX_CXX "-linux-gnueabi.so")
set(CMAKE_STATIC_LIBRARY_SUFFIX_C "-linux-gnueabi.a")
set(CMAKE_STATIC_LIBRARY_SUFFIX_CXX "-linux-gnueabi.a")


set(SDKTARGETSYSROOT "/usr/local/oecore-x86_64/sysroots/armv7at2hf-neon-oe-linux-gnueabi")
set(OECORE_NATIVE_SYSROOT "/usr/local/oecore-x86_64/sysroots/x86_64-oesdk-linux")
set(OECORE_TARGET_SYSROOT "${SDKTARGETSYSROOT}")

set(TOOLCHAIN_PREFIX "arm-oe-linux-gnueabi-")
set(ARM_TOOLCHAIN_DIR "${OECORE_NATIVE_SYSROOT}/usr/bin/arm-oe-linux-gnueabi")
set(CMAKE_C_COMPILER "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_ASM_COMPILER ${ARM_TOOLCHAIN_DIR}/${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}g++")
set(CMAKE_LINKER "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}ld ")
set(CMAKE_OBJCOPY "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_OBJDUMP "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objdump")
set(CMAKE_SIZE "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size")
set(CMAKE_AR "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}ar")

set(CMAKE_C_FLAGS "-march=armv7-a -mthumb -mfpu=neon -mfloat-abi=hard --sysroot=${SDKTARGETSYSROOT} -g -O0 -fPIC -fomit-frame-pointer -Wa,--noexecstack -fexpensive-optimizations -frename-registers -ftree-vectorize -finline-functions -finline-limit=64 -Wno-error=maybe-uninitialized -Wno-error=unused-result")
set(CMAKE_CXX_FLAGS "-march=armv7-a -mthumb -mfpu=neon -mfloat-abi=hard --sysroot=${SDKTARGETSYSROOT} -g -O0 -fPIC -fomit-frame-pointer -Wa,--noexecstack -fexpensive-optimizations -frename-registers -ftree-vectorize -finline-functions -finline-limit=64 -Wno-error=maybe-uninitialized -Wno-error=unused-result")
set(CMAKE_ASM_FLAGS "-march=armv7-a -mthumb -mfpu=neon -mfloat-abi=hard --sysroot=${SDKTARGETSYSROOT} -g -O0 -fPIC -fomit-frame-pointer -Wa,--noexecstack -fexpensive-optimizations -frename-registers -ftree-vectorize -finline-functions -finline-limit=64 -Wno-error=maybe-uninitialized -Wno-error=unused-result")
set(CMAKE_LD_FLAGS "--sysroot=${SDKTARGETSYSROOT} -g -O0 -fPIC -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed")

set(CMAKE_SHARED_LINKER_FLAGS "--sysroot=${SDKTARGETSYSROOT} -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed")
set(CMAKE_EXE_LINKER_FLAGS "--sysroot=${SDKTARGETSYSROOT} -g -O0 -fPIC -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed")

set(CMAKE_SYSROOT ${OECORE_TARGET_SYSROOT} )
if (${SDKTARGETSYSROOT} MATCHES "/sysroots/([a-zA-Z0-9_-]+)-.+-.+")
      set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_MATCH_1})
endif()

set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> -rcs --target=elf32-littlearm <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -rcs --target=elf32-littlearm <TARGET> <LINK_FLAGS> <OBJECTS>")

option(LINK_RTK "option for linking RTK" ON)
if(LINK_RTK)
add_definitions(-D_LINK_RTK)
set(RTK_SDK_LIBPATH "${PROJECT_SOURCE_DIR}/lib/qcomARMv7")
set(RTK_SDK_LIBS "liboemRtk-linux-gnueabi.a")
else()
set(RTK_SDK_LIBS "")
endif()

set(STDLIB "${RTK_SDK_LIBS}")

option(LINK_RTK_SOLVE "option for RTK solving" ON)
if(LINK_RTK_SOLVE)
add_definitions(-D_LINK_RTK_SOLVE)
endif()

option(LINK_SERIAL_PORT "option for linking serial port" OFF)
if(LINK_SERIAL_PORT)
add_definitions(-D_LINK_SERIAL_PORT)
#set(RTK_SDK_LIBPATH "${RTK_SDK_LIBPATH} ${PROJECT_SOURCE_DIR}/build/lib/qcomARMv7")
set(THIRD_PARTY_LIBS "libcserialport-linux-gnueabi.a")
#set(THIRD_PARTY_LIBS "cserialport")
else()
set(THIRD_PARTY_LIBS "")
endif()

option(RING_BUFFER_STATIC "option for allocating memory dynamically" OFF)
if(RING_BUFFER_STATIC)
add_definitions(-D_RING_BUFFER_STATIC)
endif()


option(DAT_LOG_MT "option for writing data log at another thread" ON)
if(DAT_LOG_MT)
add_definitions(-D_DAT_LOG_MT)
set(THIRD_PARTY_LIBS "${THIRD_PARTY_LIBS};pthread")
endif()

option(EC100Y_PLATFORM "option for EC100Y quectel Platform" OFF)
if(EC100Y_PLATFORM)
add_definitions(-D_EC100Y_PLATFORM)
endif()

#set(STDLIB "${STDLIB} ${THIRD_PARTY_LIBS}")
set(QL_SDK_DIR "/home/sinognss/vendor/ql-ol-extsdk-ag520rcnaar01a14m4g_ocpu_01.001.01")
set(QL_SYSROOT_DIR "${QL_SDK_DIR}/ql-sysroots")
set(QL_SDK_INCLUDE "${QL_SYSROOT_DIR}/include ${QL_SYSROOT_DIR}/usr/include ${QL_SYSROOT_DIR}/usr/include/ql_lib_utils  ${QL_SYSROOT_DIR}/usr/include/ql-sdk")
set(QL_SDK_LIBPATH "${QL_SYSROOT_DIR}/usr;${QL_SYSROOT_DIR}/usr/lib")
set(QL_SDK_LIBS "ql_sdk")


set(CMAKE_FIND_ROOT_PATH ${OECORE_TARGET_SYSROOT} ${OECORE_NATIVE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/Output/lib/qcomARMv7")

#set(CMAKE_FIND_LIBRARY_CUSTOM_LIB_SUFFIX "$ENV{OE_CMAKE_FIND_LIBRARY_CUSTOM_LIB_SUFFIX}")

