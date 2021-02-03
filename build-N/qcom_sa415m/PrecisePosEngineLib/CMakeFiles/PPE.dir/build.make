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


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /opt/cmake-3.17.2-Linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.17.2-Linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sinognss/develop/oelinuxRtk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sinognss/develop/oelinuxRtk/build-N

# Include any dependencies generated for this target.
include qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/depend.make

# Include the progress variables for this target.
include qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/progress.make

# Include the compile flags for this target's objects.
include qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/flags.make

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/PrecisePosEngine.cpp.o: qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/flags.make
qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/PrecisePosEngine.cpp.o: ../qcom_sa415m/PrecisePosEngineLib/PrecisePosEngine.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-N/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/PrecisePosEngine.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && /opt/oecore-x86_64/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/opt/oecore-x86_64/sysroots/armv7at2hf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PPE.dir/PrecisePosEngine.cpp.o -c /home/sinognss/develop/oelinuxRtk/qcom_sa415m/PrecisePosEngineLib/PrecisePosEngine.cpp

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/PrecisePosEngine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PPE.dir/PrecisePosEngine.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && /opt/oecore-x86_64/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/opt/oecore-x86_64/sysroots/armv7at2hf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/qcom_sa415m/PrecisePosEngineLib/PrecisePosEngine.cpp > CMakeFiles/PPE.dir/PrecisePosEngine.cpp.i

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/PrecisePosEngine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PPE.dir/PrecisePosEngine.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && /opt/oecore-x86_64/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/opt/oecore-x86_64/sysroots/armv7at2hf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/qcom_sa415m/PrecisePosEngineLib/PrecisePosEngine.cpp -o CMakeFiles/PPE.dir/PrecisePosEngine.cpp.s

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/ppecom.cpp.o: qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/flags.make
qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/ppecom.cpp.o: ../qcom_sa415m/PrecisePosEngineLib/ppecom.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sinognss/develop/oelinuxRtk/build-N/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/ppecom.cpp.o"
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && /opt/oecore-x86_64/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/opt/oecore-x86_64/sysroots/armv7at2hf-neon-oe-linux-gnueabi  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PPE.dir/ppecom.cpp.o -c /home/sinognss/develop/oelinuxRtk/qcom_sa415m/PrecisePosEngineLib/ppecom.cpp

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/ppecom.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PPE.dir/ppecom.cpp.i"
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && /opt/oecore-x86_64/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/opt/oecore-x86_64/sysroots/armv7at2hf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sinognss/develop/oelinuxRtk/qcom_sa415m/PrecisePosEngineLib/ppecom.cpp > CMakeFiles/PPE.dir/ppecom.cpp.i

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/ppecom.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PPE.dir/ppecom.cpp.s"
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && /opt/oecore-x86_64/sysroots/x86_64-oesdk-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-g++ --sysroot=/opt/oecore-x86_64/sysroots/armv7at2hf-neon-oe-linux-gnueabi $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sinognss/develop/oelinuxRtk/qcom_sa415m/PrecisePosEngineLib/ppecom.cpp -o CMakeFiles/PPE.dir/ppecom.cpp.s

# Object files for target PPE
PPE_OBJECTS = \
"CMakeFiles/PPE.dir/PrecisePosEngine.cpp.o" \
"CMakeFiles/PPE.dir/ppecom.cpp.o"

# External object files for target PPE
PPE_EXTERNAL_OBJECTS =

../Output/lib/qcomARMv7/libPPE-linux-gnueabi.so: qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/PrecisePosEngine.cpp.o
../Output/lib/qcomARMv7/libPPE-linux-gnueabi.so: qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/ppecom.cpp.o
../Output/lib/qcomARMv7/libPPE-linux-gnueabi.so: qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/build.make
../Output/lib/qcomARMv7/libPPE-linux-gnueabi.so: qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sinognss/develop/oelinuxRtk/build-N/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library ../../../Output/lib/qcomARMv7/libPPE-linux-gnueabi.so"
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/PPE.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/build: ../Output/lib/qcomARMv7/libPPE-linux-gnueabi.so

.PHONY : qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/build

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/clean:
	cd /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib && $(CMAKE_COMMAND) -P CMakeFiles/PPE.dir/cmake_clean.cmake
.PHONY : qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/clean

qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/depend:
	cd /home/sinognss/develop/oelinuxRtk/build-N && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sinognss/develop/oelinuxRtk /home/sinognss/develop/oelinuxRtk/qcom_sa415m/PrecisePosEngineLib /home/sinognss/develop/oelinuxRtk/build-N /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib /home/sinognss/develop/oelinuxRtk/build-N/qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : qcom_sa415m/PrecisePosEngineLib/CMakeFiles/PPE.dir/depend

