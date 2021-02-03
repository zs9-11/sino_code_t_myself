cmake_minimum_required (VERSION 2.8.0)

project(oeLinuxRtk LANGUAGES CXX C)


#add_subdirectory(util)
add_subdirectory(util-N)

string(FIND "${CMAKE_TOOLCHAIN_FILE}" "arm-none-eabi" POS)
if (${POS} GREATER -1)
add_subdirectory(ntripclient2)
else()
add_subdirectory(CSerialPort-master/lib)
add_subdirectory(ntripclient)
endif()

#add_subdirectory(OEMRtk)
add_subdirectory(OEMRtk-N)

string(FIND "${CMAKE_TOOLCHAIN_FILE}" "armv7at2hf" POS)
if (${POS} GREATER -1)
add_subdirectory(qcom_sa415m/PrecisePosEngineLib)
add_subdirectory(qcom_sa415m/PrecisePosEngineLibTest)
#add_subdirectory(qcom_9628/PrecisePosEngineLib)
endif()

string(FIND "${CMAKE_TOOLCHAIN_FILE}" "armv7ahf" POS)
if (${POS} GREATER -1)
add_subdirectory(qcom_9628/PrecisePosEngineLib)
endif()
