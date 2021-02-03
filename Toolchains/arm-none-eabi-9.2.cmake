set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_STATIC_LIBRARY_SUFFIX_C "-gcc-non.lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX_CXX "-gcc-non.lib")

set(TOOLCHAIN_PREFIX arm-none-eabi-)


set(QL_SDK_PATH "$ENV{HOME}/vendor/ql-linux-sdk/threadx")

#set(COMPILE_TOOL_PATH "$ENV{HOME}/develop/ql-linux-sdk/threadx/tools/gcc-arm-none-eabi-9-2019-q4-major")
set(COMPILE_TOOL_PATH "${QL_SDK_PATH}/tools/gcc-arm-none-eabi-9-2019-q4-major")
set(GCC_INSTALL_PATH "${COMPILE_TOOL_PATH}/bin")
set(GCC_VERSION "9.2.1")
set(GCC_STDLIB_TARGET_PATH "thumb/v7/nofp")

set(STDLIB "${COMPILE_TOOL_PATH}/arm-none-eabi/lib/$(GCC_STDLIB_TARGET_PATH)/libc.a")
set(STDLIB "${STDLIB} ${COMPILE_TOOL_PATH}/gcc/arm-none-eabi/$(GCC_VERSION)/$(GCC_STDLIB_TARGET_PATH)/libgcc.a") 
set(STDLIB "${STDLIB} ${COMPILE_TOOL_PATH}/arm-none-eabi/lib/$(GCC_STDLIB_TARGET_PATH)/libg.a")
set(STDLIB "${STDLIB} ${COMPILE_TOOL_PATH}/arm-none-eabi/lib/$(GCC_STDLIB_TARGET_PATH)/libnosys.a")
set(STDLIB "${STDLIB} ${COMPILE_TOOL_PATH}/arm-none-eabi/lib/$(GCC_STDLIB_TARGET_PATH)/libm.a") 
#set(STDLIB "${STDLIB} ${COMPILE_TOOL_PATH}/arm-none-eabi/lib/$(GCC_STDLIB_TARGET_PATH)/libstdc++.a") 

set(FNO_BUILTIN_FLAGS "-fno-builtin-printf -fno-builtin-time -fno-builtin-gmtime -fno-builtin-gettimeofday")

set(CMAKE_C_COMPILER ${GCC_INSTALL_PATH}/${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${GCC_INSTALL_PATH}/${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${GCC_INSTALL_PATH}/${TOOLCHAIN_PREFIX}g++)
set(CMAKE_LINKER "${GCC_INSTALL_PATH}/${TOOLCHAIN_PREFIX}ld ")
set(CMAKE_OBJCOPY "${GCC_INSTALL_PATH}/${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_OBJDUMP "${GCC_INSTALL_PATH}/${TOOLCHAIN_PREFIX}objdump")
set(CMAKE_SIZE "${GCC_INSTALL_PATH}/${TOOLCHAIN_PREFIX}size")
set(CMAKE_AR "${GCC_INSTALL_PATH}/${TOOLCHAIN_PREFIX}ar")


## the following is needed for CheckCSourceCompiles used in lib/CMakeLists.txt
set(CMAKE_C_FLAGS "-c -fPIC -MMD -mlong-calls -mcpu=cortex-r4 -mfloat-abi=soft -mlittle-endian -mthumb -mthumb-interwork  -Wall -ffunction-sections -fdata-sections") 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FNO_BUILTIN_FLAGS} -D__OCPU_COMPILER_GCC__ -D_WANT_USE_LONG_TIME_T -std=c99 -g -Os")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --include=${QL_SDK_PATH}/config/profile/autoconf.h")
set(CMAKE_CXX_FLAGS "-c -fPIC -MMD -mlong-calls  -mcpu=cortex-r4 -mfloat-abi=soft   -mlittle-endian -mthumb -mthumb-interwork  -Wall -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FNO_BUILTIN_FLAGS} -D__OCPU_COMPILER_GCC__ -D_WANT_USE_LONG_TIME_T -std=c++11  -g -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --include=${QL_SDK_PATH}/config/profile/autoconf.h")
set(CMAKE_ASM_FLAGS "-fPIC -c -mlong-calls  -mcpu=cortex-r4 -mfloat-abi=soft   -mlittle-endian -mthumb -mthumb-interwork  -Wall -Os")

set(CMAKE_LD_FLAGS "-fPIC -gc-sections -T${QL_SDK_PATH}/config/common/app_linkscript.ld -nostdlib")

set(CMAKE_EXE_LINKER_FLGAS_INIT "-fPIC -gc-sections -T${QL_SDK_PATH}/config/common/app_linkscript.ld -nostdlib")
set(CMAKE_EXE_LINKER_FLGAS "-fPIC -gc-sections -T${QL_SDK_PATH}/config/common/app_linkscript.ld -nostdlib")
set(CMAKE_STATIC_LINKER_FLGAS "-fPIC -gc-sections -T${QL_SDK_PATH}/config/common/app_linkscript.ld -nostdlib")
set(CMAKE_SHARED_LINKER_FLGAS "-fPIC -gc-sections -T${QL_SDK_PATH}/config/common/app_linkscript.ld -nostdlib")

set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> -rc <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -rc <TARGET> <LINK_FLAGS> <OBJECTS>")


set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})

set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/Output/lib/EC100Y")

set(GNSS_SDK_INCLUDE "${QL_SDK_PATH}/common/include;${QL_SDK_PATH}/interface/os/inc;${QL_SDK_PATH}/interface/driver/inc")
set(GNSS_SDK_INCLUDE "${GNSS_SDK_INCLUDE};${QL_SDK_PATH}/interface/network/data_call/inc;;${QL_SDK_PATH}/interface/network/sockets/inc")
set(GNSS_SDK_LIBPATH "${QL_SDK_PATH}/common/lib")
set(GNSS_SDK_LIBS "ql_common_api.lib")

option(LINK_RTK "option for linking RTK" ON)
if(LINK_RTK)
add_definitions(-D_LINK_RTK)
set(RTK_SDK_LIBPATH "${PROJECT_SOURCE_DIR}/lib/EC100Y")
set(RTK_SDK_LIBS "liboemRtk-gcc-non.lib")
else()
set(RTK_SDK_LIBS "")
endif()

set(STDLIB "${RTK_SDK_LIBS}")

option(LINK_RTK_SOLVE "option for RTK solving" ON)
#option(LINK_RTK_SOLVE "option for RTK solving" OFF)
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

option(EC100Y_PLATFORM "option for EC100Y quectel Platform" ON)
if(EC100Y_PLATFORM)
add_definitions(-D_EC100Y_PLATFORM)
set(THIRD_PARTY_LIBS "${THIRD_PARTY_LIBS};${GNSS_SDK_LIBS}")
endif()

option(DAT_LOG_MT "option for writing data log at another thread" ON)
#option(DAT_LOG_MT "option for writing data log at another thread" OFF)
if(DAT_LOG_MT)
add_definitions(-D_DAT_LOG_MT)
if(EC100Y_PLATFORM)
#set(THIRD_PARTY_LIBS "${THIRD_PARTY_LIBS};${GNSS_SDK_LIBS}")
else()
set(THIRD_PARTY_LIBS "${THIRD_PARTY_LIBS};pthread")
endif()
endif()

option(RING_BUFFER_STATIC "option for allocating memory dynamically" ON)
if(RING_BUFFER_STATIC)
add_definitions(-D_RING_BUFFER_STATIC)
endif()


