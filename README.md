Melanolib
=========

Various utilities for modern C++

Libraries
---------

### Header-only

 * geo - Basic geometry utilities
 * math/math - Misc mathematical functions
 * string/quickstream - Simple and fast string input class
 * string/string_view - A string view
 * string/trie - Prefix tree data structure
 * utils - Miscellaneous utilities

### Compiled

 * dynlib - Utilities to load dynamic library at runtime (Currently only implemented for POSIX)
 * math/random - Simple uniform random number generation
 * stringutils - Various string functions and English inflection
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
