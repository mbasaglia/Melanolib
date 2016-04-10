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
cmake_minimum_required(VERSION 2.6)

# Metadata
project(MelanoLib CXX)
set(PROJECT_VERSION 0.1)

# Check C++14
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_VERSION} LESS 3.2)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag(--std=c++${CMAKE_CXX_STANDARD} STD_CXX)
        if(STD_CXX)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++${CMAKE_CXX_STANDARD}")
        else()
            message(SEND_ERROR "Requires C++${CMAKE_CXX_STANDARD} or better")
        endif()
    else()
        message(WARNING "Unrecognized compiler: ${CMAKE_CXX_COMPILER_ID}, make sure it supports C++${CMAKE_CXX_STANDARD}")
    endif()
endif()

# Flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -Werror")

# Enable Debug by default, can be changed with -D CMAKE_BUILD_TYPE=Release
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(src)

include(testing.cmake)
add_subdirectory(test)
