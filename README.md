Melanolib
=========

Various utilities for modern C++

Libraries
---------

### Header-only

 * dtat_structures - Various data structures and supporting functions
 * geo - Basic geometry utilities
 * math - Misc mathematical functions and types
 * scripting - Infrastructure to implement scripting languages
 * string/ascii - Ascii character classification
 * string/quickstream - Simple and fast string input class
 * string/simple_stringutils - Various string functions
 * string/string_view - A string view
 * string/trie - Prefix tree data structure
 * utils - Miscellaneous utilities

### Compiled

 * color - Color class with different color spaces (only Color::format needs to link)
 * dynlib - Utilities to load dynamic library at runtime (Currently only implemented for POSIX)
 * stringutils - Various string functions, UTF8 support, and natural language generation
 * time - Date and time utilities

Configuration
-------------

You can force the library to use the boost version of `any` and `optional`
by defining the macros `MELANOLIB_BOOST_ANY` and `MELANOLIB_BOOST_OPTIONAL`.

Compiling
---------

    mkdir build && cd build && cmake .. && make

Authors
-------

Mattia Basaglia <mattia.basaglia@gmail.com>


License
-------
GPLv3 or later, see COPYING.


Sources
-------

Up to date sources are available at https://github.com/mbasaglia/Melanolib
