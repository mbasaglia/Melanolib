/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2015-2016 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "melanolib/dynlib/library.hpp"
#include "library_name.hpp"
#include "melanolib/string/simple_stringutils.hpp"

#if __has_include(<dlfcn.h>)
#include <dlfcn.h>

class melanolib::dynlib::Library::Private
{
public:
    void* handle = nullptr;
    const char* error_string = nullptr;
    std::string filename;

    void gather_error()
    {
        error_string = dlerror();
    }

    void close()
    {
        if ( handle )
            dlclose(handle);
    }

    void open(LoadFlags flags)
    {
        bool throws = flags & LoadThrows;
        flags &= ~LoadThrows;
        handle = dlopen(filename.c_str(), flags);
        if ( !handle )
        {
            gather_error();
            if ( throws )
                throw LibraryError(filename, error_string);
        }
    }

    void* resolve(const std::string& symbol)
    {
        auto ret = dlsym(handle, symbol.c_str());
        if ( !ret )
            gather_error();
        return ret;
    }

    bool has_handle() const
    {
        return handle;
    }
};

#else

class melanolib::dynlib::Library::Private
{
public:
    void* handle = nullptr;
    const char* error_string = "Dynamic library loading has not been implemented for this system";
    std::string filename;

    void close()
    {
    }

    void open(LoadFlags flags)
    {
        if ( flags & LoadThrows )
            throw LibraryError(filename, error_string);
    }

    void* resolve(const std::string& symbol)
    {
        return nullptr;
    }

    bool has_handle() const
    {
        return false;
    }
};

#endif

namespace melanolib {
namespace dynlib {

Library::Library(const std::string& library_file, LoadFlags flags)
    : p(std::make_shared<Private>())
{
    p->filename = library_file;
    p->open(flags);
}

Library::~Library()
{
    if ( p.unique() )
        p->close();
}

bool Library::error() const
{
    return !p->has_handle() || p->error_string;
}

bool Library::fatal_error() const
{
    return !p->has_handle();
}

std::string Library::error_string() const
{
    return p->error_string ? p->error_string : "";
}

std::string Library::filename() const
{
    return p->filename;
}

void* Library::resolve_raw(const std::string& name) const
{
    return p->resolve(name);
}


void Library::reload(dynlib::LoadFlags flags) const
{
    p->close();
    p->open(flags);
}

std::string Library::library_prefix()
{
    return LIB_PREFIX;
}

std::string Library::library_suffix()
{
    return LIB_SUFFIX;
}

bool Library::is_library_basename(const std::string& name)
{
    return name.size() > LIB_PREFIX.size() + LIB_SUFFIX.size() &&
           string::starts_with(name, LIB_PREFIX) &&
           string::ends_with(name, LIB_SUFFIX);
}

std::string Library::library_name(std::string basename)
{
    if ( !is_library_basename(basename) )
        return "";

    basename.erase(0, LIB_PREFIX.size());
    basename.erase(basename.size() - LIB_SUFFIX.size());

    return basename;
}



} // namespace dynlib
} // namespace melanolib
