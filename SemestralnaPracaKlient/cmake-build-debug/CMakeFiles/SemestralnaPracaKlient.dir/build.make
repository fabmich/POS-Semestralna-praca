# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/tmp.X1uWlasjxx

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.X1uWlasjxx/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/SemestralnaPracaKlient.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/SemestralnaPracaKlient.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/SemestralnaPracaKlient.dir/flags.make

CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.o: CMakeFiles/SemestralnaPracaKlient.dir/flags.make
CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.X1uWlasjxx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.o -c /tmp/tmp.X1uWlasjxx/main.cpp

CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.X1uWlasjxx/main.cpp > CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.i

CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.X1uWlasjxx/main.cpp -o CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.s

CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.o: CMakeFiles/SemestralnaPracaKlient.dir/flags.make
CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.o: ../my_socket.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.X1uWlasjxx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.o -c /tmp/tmp.X1uWlasjxx/my_socket.cpp

CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.X1uWlasjxx/my_socket.cpp > CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.i

CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.X1uWlasjxx/my_socket.cpp -o CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.s

# Object files for target SemestralnaPracaKlient
SemestralnaPracaKlient_OBJECTS = \
"CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.o" \
"CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.o"

# External object files for target SemestralnaPracaKlient
SemestralnaPracaKlient_EXTERNAL_OBJECTS =

SemestralnaPracaKlient: CMakeFiles/SemestralnaPracaKlient.dir/main.cpp.o
SemestralnaPracaKlient: CMakeFiles/SemestralnaPracaKlient.dir/my_socket.cpp.o
SemestralnaPracaKlient: CMakeFiles/SemestralnaPracaKlient.dir/build.make
SemestralnaPracaKlient: CMakeFiles/SemestralnaPracaKlient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.X1uWlasjxx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable SemestralnaPracaKlient"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SemestralnaPracaKlient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/SemestralnaPracaKlient.dir/build: SemestralnaPracaKlient

.PHONY : CMakeFiles/SemestralnaPracaKlient.dir/build

CMakeFiles/SemestralnaPracaKlient.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/SemestralnaPracaKlient.dir/cmake_clean.cmake
.PHONY : CMakeFiles/SemestralnaPracaKlient.dir/clean

CMakeFiles/SemestralnaPracaKlient.dir/depend:
	cd /tmp/tmp.X1uWlasjxx/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.X1uWlasjxx /tmp/tmp.X1uWlasjxx /tmp/tmp.X1uWlasjxx/cmake-build-debug /tmp/tmp.X1uWlasjxx/cmake-build-debug /tmp/tmp.X1uWlasjxx/cmake-build-debug/CMakeFiles/SemestralnaPracaKlient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/SemestralnaPracaKlient.dir/depend

