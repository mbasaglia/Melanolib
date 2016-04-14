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

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(NOT DEFINED BUILD_SHARED_MELANO_LIBS)
    set(BUILD_SHARED_MELANO_LIBS ${BUILD_SHARED_LIBS})
endif()

function(add_melano_lib name)
    if(${BUILD_SHARED_MELANO_LIBS})
        add_library(${name} SHARED ${ARGN})
    else()
        add_library(${name} STATIC ${ARGN})
    endif()
endfunction()

add_library(melano INTERFACE)
target_link_libraries(melano INTERFACE melano_random melano_dynlib melano_stringutils melano_time)


add_subdirectory(math)
add_subdirectory(string)
add_subdirectory(time)
add_subdirectory(dynlib)
