# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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
CMAKE_SOURCE_DIR = /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/build

# Include any dependencies generated for this target.
include CMakeFiles/exodia.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/exodia.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/exodia.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/exodia.dir/flags.make

CMakeFiles/exodia.dir/src/main.c.o: CMakeFiles/exodia.dir/flags.make
CMakeFiles/exodia.dir/src/main.c.o: /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/src/main.c
CMakeFiles/exodia.dir/src/main.c.o: CMakeFiles/exodia.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/exodia.dir/src/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/exodia.dir/src/main.c.o -MF CMakeFiles/exodia.dir/src/main.c.o.d -o CMakeFiles/exodia.dir/src/main.c.o -c /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/src/main.c

CMakeFiles/exodia.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/exodia.dir/src/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/src/main.c > CMakeFiles/exodia.dir/src/main.c.i

CMakeFiles/exodia.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/exodia.dir/src/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/src/main.c -o CMakeFiles/exodia.dir/src/main.c.s

# Object files for target exodia
exodia_OBJECTS = \
"CMakeFiles/exodia.dir/src/main.c.o"

# External object files for target exodia
exodia_EXTERNAL_OBJECTS =

exodia: CMakeFiles/exodia.dir/src/main.c.o
exodia: CMakeFiles/exodia.dir/build.make
exodia: CMakeFiles/exodia.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable exodia"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/exodia.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/exodia.dir/build: exodia
.PHONY : CMakeFiles/exodia.dir/build

CMakeFiles/exodia.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/exodia.dir/cmake_clean.cmake
.PHONY : CMakeFiles/exodia.dir/clean

CMakeFiles/exodia.dir/depend:
	cd /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/build /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/build /mnt/c/Users/chris/OneDrive/Documents/Code/C-C++/Exodia/build/CMakeFiles/exodia.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/exodia.dir/depend

