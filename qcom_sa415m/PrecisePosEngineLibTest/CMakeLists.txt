cmake_minimum_required (VERSION 3.15)

project ("PrecisePosEngineLibTest")

include_directories(../EnginePluginAPI)

link_directories(${RTK_SDK_LIBPATH})
link_directories(${LIBRARY_OUTPUT_PATH})

aux_source_directory(. SRC_DIR)

add_executable(ppe_test ${SRC_DIR})
target_link_libraries(ppe_test PPE util-N-linux-gnueabi PreRtk-N-linux-gnueabi ${STDLIB} dl ${THIRD_PARTY_LIBS})  
