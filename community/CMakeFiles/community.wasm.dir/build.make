# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.11.4/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.11.4/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/vohongquy/can_project/governance-design/community/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/vohongquy/can_project/governance-design/community

# Include any dependencies generated for this target.
include CMakeFiles/community.wasm.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/community.wasm.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/community.wasm.dir/flags.make

CMakeFiles/community.wasm.dir/code.cpp.o: CMakeFiles/community.wasm.dir/flags.make
CMakeFiles/community.wasm.dir/code.cpp.o: src/code.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/vohongquy/can_project/governance-design/community/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/community.wasm.dir/code.cpp.o"
	//usr/local/bin/eosio-cpp  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/community.wasm.dir/code.cpp.o -c /Users/vohongquy/can_project/governance-design/community/src/code.cpp

CMakeFiles/community.wasm.dir/code.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/community.wasm.dir/code.cpp.i"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vohongquy/can_project/governance-design/community/src/code.cpp > CMakeFiles/community.wasm.dir/code.cpp.i

CMakeFiles/community.wasm.dir/code.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/community.wasm.dir/code.cpp.s"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vohongquy/can_project/governance-design/community/src/code.cpp -o CMakeFiles/community.wasm.dir/code.cpp.s

CMakeFiles/community.wasm.dir/position.cpp.o: CMakeFiles/community.wasm.dir/flags.make
CMakeFiles/community.wasm.dir/position.cpp.o: src/position.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/vohongquy/can_project/governance-design/community/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/community.wasm.dir/position.cpp.o"
	//usr/local/bin/eosio-cpp  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/community.wasm.dir/position.cpp.o -c /Users/vohongquy/can_project/governance-design/community/src/position.cpp

CMakeFiles/community.wasm.dir/position.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/community.wasm.dir/position.cpp.i"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vohongquy/can_project/governance-design/community/src/position.cpp > CMakeFiles/community.wasm.dir/position.cpp.i

CMakeFiles/community.wasm.dir/position.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/community.wasm.dir/position.cpp.s"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vohongquy/can_project/governance-design/community/src/position.cpp -o CMakeFiles/community.wasm.dir/position.cpp.s

CMakeFiles/community.wasm.dir/badge.cpp.o: CMakeFiles/community.wasm.dir/flags.make
CMakeFiles/community.wasm.dir/badge.cpp.o: src/badge.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/vohongquy/can_project/governance-design/community/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/community.wasm.dir/badge.cpp.o"
	//usr/local/bin/eosio-cpp  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/community.wasm.dir/badge.cpp.o -c /Users/vohongquy/can_project/governance-design/community/src/badge.cpp

CMakeFiles/community.wasm.dir/badge.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/community.wasm.dir/badge.cpp.i"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vohongquy/can_project/governance-design/community/src/badge.cpp > CMakeFiles/community.wasm.dir/badge.cpp.i

CMakeFiles/community.wasm.dir/badge.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/community.wasm.dir/badge.cpp.s"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vohongquy/can_project/governance-design/community/src/badge.cpp -o CMakeFiles/community.wasm.dir/badge.cpp.s

CMakeFiles/community.wasm.dir/community.cpp.o: CMakeFiles/community.wasm.dir/flags.make
CMakeFiles/community.wasm.dir/community.cpp.o: src/community.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/vohongquy/can_project/governance-design/community/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/community.wasm.dir/community.cpp.o"
	//usr/local/bin/eosio-cpp  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/community.wasm.dir/community.cpp.o -c /Users/vohongquy/can_project/governance-design/community/src/community.cpp

CMakeFiles/community.wasm.dir/community.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/community.wasm.dir/community.cpp.i"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vohongquy/can_project/governance-design/community/src/community.cpp > CMakeFiles/community.wasm.dir/community.cpp.i

CMakeFiles/community.wasm.dir/community.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/community.wasm.dir/community.cpp.s"
	//usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vohongquy/can_project/governance-design/community/src/community.cpp -o CMakeFiles/community.wasm.dir/community.cpp.s

# Object files for target community.wasm
community_wasm_OBJECTS = \
"CMakeFiles/community.wasm.dir/code.cpp.o" \
"CMakeFiles/community.wasm.dir/position.cpp.o" \
"CMakeFiles/community.wasm.dir/badge.cpp.o" \
"CMakeFiles/community.wasm.dir/community.cpp.o"

# External object files for target community.wasm
community_wasm_EXTERNAL_OBJECTS =

community.wasm: CMakeFiles/community.wasm.dir/code.cpp.o
community.wasm: CMakeFiles/community.wasm.dir/position.cpp.o
community.wasm: CMakeFiles/community.wasm.dir/badge.cpp.o
community.wasm: CMakeFiles/community.wasm.dir/community.cpp.o
community.wasm: CMakeFiles/community.wasm.dir/build.make
community.wasm: CMakeFiles/community.wasm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/vohongquy/can_project/governance-design/community/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable community.wasm"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/community.wasm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/community.wasm.dir/build: community.wasm

.PHONY : CMakeFiles/community.wasm.dir/build

CMakeFiles/community.wasm.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/community.wasm.dir/cmake_clean.cmake
.PHONY : CMakeFiles/community.wasm.dir/clean

CMakeFiles/community.wasm.dir/depend:
	cd /Users/vohongquy/can_project/governance-design/community && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vohongquy/can_project/governance-design/community/src /Users/vohongquy/can_project/governance-design/community/src /Users/vohongquy/can_project/governance-design/community /Users/vohongquy/can_project/governance-design/community /Users/vohongquy/can_project/governance-design/community/CMakeFiles/community.wasm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/community.wasm.dir/depend

