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

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/library_name.in.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/library_name.hpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_melano_lib(melano_dynlib SHARED library.cpp)
target_link_libraries(melano_dynlib ${CMAKE_DL_LIBS})
