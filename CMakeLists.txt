# Copyright 2015-2016 Mattia Basaglia
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# CMake stuff
cmake_minimum_required(VERSION 3.0)

# Metadata
project(MelanoLib LANGUAGES CXX VERSION 0.1)

# CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(misc)

# Check C++14
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
use_cxx_standard(14)

# Flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -Werror --coverage")

# Enable Debug by default, can be changed with -D CMAKE_BUILD_TYPE=Release
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(src)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Tests
    set(LCOV_SOURCE_BASE_DIR "${CMAKE_SOURCE_DIR}")
    set(LCOV_REMOVE_PATTERN "test/*.cpp")
    include(testing)
    add_subdirectory(test)


    # Doxygen target
    # Find all sources for documentation and stuff
    set(ALL_SOURCE_DIRECTORIES src include)

    find_sources(ALL_SOURCES *.cpp *.hpp)
    set(DOXYGEN_FOUND ON)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        create_doxygen_target(doc)
    endif(DOXYGEN_FOUND)
endif()
