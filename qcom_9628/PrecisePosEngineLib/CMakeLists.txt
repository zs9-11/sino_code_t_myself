cmake_minimum_required (VERSION 3.15)

project ("PrecisePosEngineLib")

option(_DEBUG "option for debug" OFF)
if(_DEBUG)
add_definitions(-D_DEBUG)
endif()

include_directories(../../util-N)
include_directories(../EnginePluginAPI)
include_directories(../../OEMRtk-N)
include_directories(../../ntripclient)
include_directories(../../CSerialPort-master/src)

link_directories(${RTK_SDK_LIBPATH})
link_directories(${LIBRARY_OUTPUT_PATH})

aux_source_directory(. SRC_DIR)

add_library (PPE-9628 SHARED ${SRC_DIR})
#target_link_libraries(PPE-9628 util-linux-gnueabi cserialport-linux-gnueabi PreRtk-linux-gnueabi ${STDLIB})  
target_link_libraries(PPE-9628 util-N-linux-gnueabi cserialport-linux-gnueabi PreRtk-N-linux-gnueabi ${STDLIB})  
