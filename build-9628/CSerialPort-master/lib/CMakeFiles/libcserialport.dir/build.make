# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake-3.17.3-Linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.17.3-Linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sinognss/develop/oelinuxRtk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sinognss/develop/oelinuxRtk/build-9628

# Include any dependencies generated for this target.
include CSerialPort-master/lib/CMakeFiles/libcserialport.dir/depend.make

# Include the progress variables for this target.
include CSerialPort-master/lib/CMakeFiles/libcserialport.dir/progress.make

# Include the compile flags for this target's objects.
include CSerialPort-master/lib/CMakeFiles/libcserialport.dir/flags.make

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.o: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/flags.make
CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.o: ../CSerialPort-master/src/SerialPort.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-9628/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.o -c /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPort.cpp

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPort.cpp > CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.i

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPort.cpp -o CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.s

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.o: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/flags.make
CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.o: ../CSerialPort-master/src/SerialPortBase.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-9628/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.o -c /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortBase.cpp

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortBase.cpp > CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.i

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortBase.cpp -o CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.s

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.o: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/flags.make
CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.o: ../CSerialPort-master/src/SerialPortInfo.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-9628/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.o -c /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfo.cpp

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfo.cpp > CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.i

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfo.cpp -o CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.s

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.o: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/flags.make
CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.o: ../CSerialPort-master/src/SerialPortInfoBase.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-9628/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.o -c /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfoBase.cpp

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfoBase.cpp > CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.i

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfoBase.cpp -o CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.s

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.o: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/flags.make
CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.o: ../CSerialPort-master/src/SerialPortInfoUnixBase.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-9628/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.o -c /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfoUnixBase.cpp

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfoUnixBase.cpp > CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.i

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortInfoUnixBase.cpp -o CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.s

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.o: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/flags.make
CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.o: ../CSerialPort-master/src/SerialPortUnixBase.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-9628/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.o -c /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortUnixBase.cpp

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortUnixBase.cpp > CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.i

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && /home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/home/sinognss/vendor/A70B_OPEN_LINUX_Q220_SDK_V3.33/tool/neoway-arm-oe-linux/sysroots/armv7ahf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/CSerialPort-master/src/SerialPortUnixBase.cpp -o CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.s

# Object files for target libcserialport
libcserialport_OBJECTS = \
"CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.o" \
"CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.o" \
"CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.o" \
"CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.o" \
"CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.o" \
"CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.o"

# External object files for target libcserialport
libcserialport_EXTERNAL_OBJECTS =

../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPort.cpp.o
../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortBase.cpp.o
../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfo.cpp.o
../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoBase.cpp.o
../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortInfoUnixBase.cpp.o
../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/__/src/SerialPortUnixBase.cpp.o
../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/build.make
../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a: CSerialPort-master/lib/CMakeFiles/libcserialport.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sinognss/develop/oelinuxRtk/build-9628/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX static library ../../../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a"
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && $(CMAKE_COMMAND) -P CMakeFiles/libcserialport.dir/cmake_clean_target.cmake
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libcserialport.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CSerialPort-master/lib/CMakeFiles/libcserialport.dir/build: ../Output/lib/qcom_9628/libcserialport-linux-gnueabi.a

.PHONY : CSerialPort-master/lib/CMakeFiles/libcserialport.dir/build

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/clean:
	cd /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib && $(CMAKE_COMMAND) -P CMakeFiles/libcserialport.dir/cmake_clean.cmake
.PHONY : CSerialPort-master/lib/CMakeFiles/libcserialport.dir/clean

CSerialPort-master/lib/CMakeFiles/libcserialport.dir/depend:
	cd /home/sinognss/develop/oelinuxRtk/build-9628 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sinognss/develop/oelinuxRtk /home/sinognss/develop/oelinuxRtk/CSerialPort-master/lib /home/sinognss/develop/oelinuxRtk/build-9628 /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib /home/sinognss/develop/oelinuxRtk/build-9628/CSerialPort-master/lib/CMakeFiles/libcserialport.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CSerialPort-master/lib/CMakeFiles/libcserialport.dir/depend

