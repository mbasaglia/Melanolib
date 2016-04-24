/**
 * \file
 * \author Mattia Basaglia
 * \copyright Copyright 2015-2016 Mattia Basaglia
 * \section License
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE Test_Dynlib
#include <boost/test/unit_test.hpp>

#include "melanolib/dynlib/library.hpp"

using namespace melanolib::dynlib;

BOOST_AUTO_TEST_CASE( test_library_load_success )
{
    Library lib(LIB_FILE, LoadNow|DeepBind|LoadThrows);

    BOOST_CHECK( lib.filename() == LIB_FILE );
    BOOST_CHECK( !lib.error() );
    BOOST_CHECK( lib.error_string().empty() );
    BOOST_CHECK( !lib.fatal_error() );
    BOOST_CHECK( bool(lib) );
}

BOOST_AUTO_TEST_CASE( test_library_resolve_success )
{
    Library lib(LIB_FILE, LoadNow|DeepBind|LoadThrows);

    auto foobar = lib.resolve_function<int (int)>("foobar");
    BOOST_CHECK( foobar );
    BOOST_CHECK( foobar(5) == 6 );
    BOOST_CHECK( lib.call_function<int>("foobar", 7) == 8 );

    BOOST_CHECK( lib.resolve_global<int>("global") == 2 );
}

BOOST_AUTO_TEST_CASE( test_library_load_error )
{
    auto load = [](int flags) {
        return Library("wrong_" LIB_FILE "_does_not_exist", LoadNow|DeepBind|flags);
    };

    BOOST_REQUIRE_THROW( load(LoadThrows), LibraryError );
    BOOST_REQUIRE_NO_THROW( load(0) );
    Library lib = load(0);
    BOOST_CHECK( lib.error() );
    BOOST_CHECK( lib.fatal_error() );
    BOOST_CHECK( !bool(lib) );
    BOOST_CHECK( !lib.error_string().empty() );
}

BOOST_AUTO_TEST_CASE( test_library_resolve_error )
{
    Library lib(LIB_FILE, LoadNow|DeepBind|LoadThrows);

    BOOST_REQUIRE_THROW( lib.resolve_function<int (int)>("fubar"), SymbolNotFoundError );
    BOOST_CHECK( lib.error() );
    BOOST_CHECK( !lib.fatal_error() );
    BOOST_CHECK( bool(lib) );
    BOOST_CHECK( !lib.error_string().empty() );
    BOOST_REQUIRE_THROW( lib.call_function<int>("fubar", 7), SymbolNotFoundError );
    BOOST_REQUIRE_THROW( lib.resolve_global<int>("fubar"), SymbolNotFoundError );

    BOOST_CHECK( lib.resolve_global<int>("global") == 2 );
}

BOOST_AUTO_TEST_CASE( test_library_static )
{
    /// \todo Somehow make this test portable
    BOOST_CHECK_EQUAL(Library::library_prefix(), "lib");
    BOOST_CHECK_EQUAL(Library::library_suffix(), ".so");
    BOOST_CHECK(!Library::is_library_basename("lib.so"));
    BOOST_CHECK(Library::is_library_basename("libfoo.so"));
    BOOST_CHECK(!Library::is_library_basename("lubfoo.so"));
    BOOST_CHECK(!Library::is_library_basename("libfoo.sup"));
    BOOST_CHECK_EQUAL(Library::library_name("libfoo.so"), "foo");
    BOOST_CHECK_EQUAL(Library::library_name("libfoo.sup"), "");
}
