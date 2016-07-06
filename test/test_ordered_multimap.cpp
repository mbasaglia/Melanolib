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
#define BOOST_TEST_MODULE Test_OrderedMultiMap
#include <boost/test/unit_test.hpp>

#include "melanolib/data_structures/ordered_multimap.hpp"

using namespace melanolib;

BOOST_AUTO_TEST_CASE( test_default_comparators )
{
    using Map = OrderedMultimap<>;
    Map map;
    Map::value_type a("foo", "bar");
    Map::value_type b("foo", "baz");
    Map::value_type c("food", "bar");
    Map::value_type d("Foo", "bar");
    Map::value_type e("foo", "Bar");

    BOOST_CHECK( map.value_comp()(a, a) );
    BOOST_CHECK( !map.value_comp()(a, b) );
    BOOST_CHECK( !map.value_comp()(a, c) );
    BOOST_CHECK( !map.value_comp()(a, d) );
    BOOST_CHECK( !map.value_comp()(a, e) );

    BOOST_CHECK( map.key_comp()(a.first, a.first) );
    BOOST_CHECK( map.key_comp()(a.first, b.first) );
    BOOST_CHECK( !map.key_comp()(a.first, c.first) );
    BOOST_CHECK( !map.key_comp()(a.first, d.first) );
    BOOST_CHECK( map.key_comp()(a.first, e.first) );

    BOOST_CHECK( map.value_comp().key(a.first, a.first) );
    BOOST_CHECK( map.value_comp().key(a.first, b.first) );
    BOOST_CHECK( !map.value_comp().key(a.first, c.first) );
    BOOST_CHECK( !map.value_comp().key(a.first, d.first) );
    BOOST_CHECK( map.value_comp().key(a.first, e.first) );

    BOOST_CHECK( map.value_comp().value(a.second, a.second) );
    BOOST_CHECK( !map.value_comp().value(a.second, b.second) );
    BOOST_CHECK( map.value_comp().value(a.second, c.second) );
    BOOST_CHECK( map.value_comp().value(a.second, d.second) );
    BOOST_CHECK( !map.value_comp().value(a.second, e.second) );
}

BOOST_AUTO_TEST_CASE( test_icase_comparators )
{
    using Map = OrderedMultimap<std::string, std::string, ICaseComparator>;
    Map map;
    Map::value_type a("foo", "bar");
    Map::value_type b("foo", "baz");
    Map::value_type c("food", "bar");
    Map::value_type d("Foo", "bar");
    Map::value_type e("foo", "Bar");

    BOOST_CHECK( map.value_comp()(a, a) );
    BOOST_CHECK( !map.value_comp()(a, b) );
    BOOST_CHECK( !map.value_comp()(a, c) );
    BOOST_CHECK( map.value_comp()(a, d) );
    BOOST_CHECK( !map.value_comp()(a, e) );

    BOOST_CHECK( map.key_comp()(a.first, a.first) );
    BOOST_CHECK( map.key_comp()(a.first, b.first) );
    BOOST_CHECK( !map.key_comp()(a.first, c.first) );
    BOOST_CHECK( map.key_comp()(a.first, d.first) );
    BOOST_CHECK( map.key_comp()(a.first, e.first) );

    BOOST_CHECK( map.value_comp().key(a.first, a.first) );
    BOOST_CHECK( map.value_comp().key(a.first, b.first) );
    BOOST_CHECK( !map.value_comp().key(a.first, c.first) );
    BOOST_CHECK( map.value_comp().key(a.first, d.first) );
    BOOST_CHECK( map.value_comp().key(a.first, e.first) );

    BOOST_CHECK( map.value_comp().value(a.second, a.second) );
    BOOST_CHECK( !map.value_comp().value(a.second, b.second) );
    BOOST_CHECK( map.value_comp().value(a.second, c.second) );
    BOOST_CHECK( map.value_comp().value(a.second, d.second) );
    BOOST_CHECK( !map.value_comp().value(a.second, e.second) );
}

BOOST_AUTO_TEST_CASE( test_ctor )
{
    using Map = OrderedMultimap<>;
    Map::container_type vector {{"", ""}, {"", ""}, {"", ""}, {"", ""}};
    Map::key_compare key_comp;
    BOOST_CHECK( Map().empty() );
    BOOST_CHECK( Map(key_comp).empty() );
    BOOST_CHECK( Map(vector).size() == vector.size() );
    BOOST_CHECK( Map(vector, key_comp).size() == vector.size() );
    BOOST_CHECK( Map({{"", ""}, {"", ""}, {"", ""}}).size() == 3 );
    BOOST_CHECK( Map({{"", ""}, {"", ""}, {"", ""}}, key_comp).size() == 3 );
    BOOST_CHECK( Map(vector.begin(), vector.end()).size() == vector.size() );
    BOOST_CHECK( Map(vector.begin(), vector.end(), key_comp).size() == vector.size() );
}

BOOST_AUTO_TEST_CASE( test_assign_op )
{
    using Map = OrderedMultimap<>;
    Map::container_type vector {{"", ""}, {"", ""}, {"", ""}, {"", ""}};
    Map map;
    BOOST_CHECK( (map = {{"", ""}, {"", ""}, {"", ""}}).size() == 3 );
}

BOOST_AUTO_TEST_CASE( test_element_access )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    const Map b = a;

    BOOST_CHECK( a.at("foo") == "1" );
    BOOST_CHECK( (std::is_same<decltype(a.at("foo")), Map::mapped_type&>::value) );
    BOOST_CHECK_THROW( a.at("foobar"), std::out_of_range );
    BOOST_CHECK( a["foo"] == "1" );
    BOOST_CHECK( (std::is_same<decltype(a["foo"]), Map::mapped_type&>::value) );
    BOOST_CHECK( a.get("foo") == "1" );
    BOOST_CHECK( a.front().second == a["foo"] );
    BOOST_CHECK( a.back().second == a["bar"] );

    BOOST_CHECK( b.at("foo") == "1" );
    BOOST_CHECK( (std::is_same<decltype(b.at("foo")), const Map::mapped_type&>::value) );
    BOOST_CHECK_THROW( b.at("foobar"), std::out_of_range );
    BOOST_CHECK( b["foo"] == "1" );
    BOOST_CHECK( !(std::is_same<decltype(b["foo"]), Map::mapped_type&>::value) );
    BOOST_CHECK( b.get("foo") == "1" );
    BOOST_CHECK( b.front().second == b["foo"] );
    BOOST_CHECK( b.back().second == b["bar"] );
}

BOOST_AUTO_TEST_CASE( test_element_access_miss )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    const Map b = a;

    BOOST_CHECK( a["foobar"] == "" );
    BOOST_CHECK( a.size() == 3 );

    BOOST_CHECK( b["foobar"] == "" );
    BOOST_CHECK( b.size() == 2 );

    BOOST_CHECK( a.get("123") == "" );
    BOOST_CHECK( a.size() == 3 );
}

BOOST_AUTO_TEST_CASE( test_iterators )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    const Map& const_a = a;

    BOOST_CHECK( a.begin() != a.end() );
    BOOST_CHECK( a.begin() == const_a.begin() );
    BOOST_CHECK( a.cbegin() == const_a.begin() );
    BOOST_CHECK( a.end() == const_a.end() );
    BOOST_CHECK( a.end() == const_a.end() );

    BOOST_CHECK( a.rbegin() != a.rend() );
    BOOST_CHECK( a.rbegin() == const_a.rbegin() );
    BOOST_CHECK( a.crbegin() == const_a.rbegin() );
    BOOST_CHECK( a.rend() == const_a.rend() );
    BOOST_CHECK( a.rend() == const_a.rend() );
    BOOST_CHECK( a.rbegin().base() == a.end() );
    BOOST_CHECK( a.rend().base() == a.begin() );
}

BOOST_AUTO_TEST_CASE( test_size )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    Map b;

    BOOST_CHECK( a.size() == 2 );
    BOOST_CHECK( !a.empty() );
    BOOST_CHECK( a.size() < a.max_size() );

    BOOST_CHECK( b.size() == 0 );
    BOOST_CHECK( b.empty() );
    BOOST_CHECK( b.max_size() == a.max_size() );
}

BOOST_AUTO_TEST_CASE( test_cmp_op )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    Map b = a;
    Map c = {{"foo", "1"}, {"bar", "2"}, {"baz", "3"}};

    BOOST_CHECK( a == b );
    BOOST_CHECK( !(a == c) );
    BOOST_CHECK( !(a != b) );
    BOOST_CHECK( a != c );
}

// TODO: insert emplace erase

BOOST_AUTO_TEST_CASE( test_erase_key )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}, {"baz", "3"}, {"foo", "4"}};
    BOOST_CHECK( a.erase("foo") == 2 );
    BOOST_CHECK( a == Map({{"bar", "2"}, {"baz", "3"}}) );
    BOOST_CHECK( a.erase("foo") == 0 );
    BOOST_CHECK( a == Map({{"bar", "2"}, {"baz", "3"}}) );
}

BOOST_AUTO_TEST_CASE( test_clear )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}, {"baz", "3"}, {"foo", "4"}};
    a.clear();
    BOOST_CHECK( a.empty() );
}

BOOST_AUTO_TEST_CASE( test_swap )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    Map b = {{"foobar", "3"}};
    Map orig_a = a;
    Map orig_b = b;
    a.swap(b);
    BOOST_CHECK( a == orig_b );
    BOOST_CHECK( b == orig_a );
}

BOOST_AUTO_TEST_CASE( test_count )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}, {"foo", "3"}};
    BOOST_CHECK( a.count("foo") == 2 );
    BOOST_CHECK( a.count("bar") == 1 );
    BOOST_CHECK( a.count("foobar") == 0 );
}

BOOST_AUTO_TEST_CASE( test_find )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}, {"foo", "3"}};
    const Map& b = a;
    BOOST_CHECK( a.find("foo") == a.begin() );
    BOOST_CHECK( a.find("foobar") == a.end() );

    BOOST_CHECK( b.find("foo") == a.cbegin() );
    BOOST_CHECK( b.find("foobar") == a.cend() );
}

BOOST_AUTO_TEST_CASE( test_icase_cmp_op )
{
    using Map = OrderedMultimap<std::string, std::string, ICaseComparator>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    Map b = {{"Foo", "1"}, {"bar", "2"}};
    Map c = {{"foo", "1"}, {"bar", "2"}, {"baz", "3"}};

    BOOST_CHECK( a == b );
    BOOST_CHECK( !(a == c) );
    BOOST_CHECK( !(a != b) );
    BOOST_CHECK( a != c );
}

BOOST_AUTO_TEST_CASE( test_contains )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}, {"foo", "3"}};
    BOOST_CHECK( a.contains("foo") );
    BOOST_CHECK( !a.contains("foobar") );
}

BOOST_AUTO_TEST_CASE( test_append )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}};
    Map b = {{"foo", "1"}, {"bar", "2"}, {"foo", "3"}};
    a.append("foo", "3");
    BOOST_CHECK( a == b );
}

BOOST_AUTO_TEST_CASE( test_key_iterator )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}, {"foo", "3"}};

    BOOST_CHECK( a.key_begin("foo") == a.begin() );
    BOOST_CHECK( a.key_begin("foo") != a.end() );
    BOOST_CHECK( a.key_end("foo") == a.end() );
    auto iter = a.key_begin("foo");
    BOOST_CHECK_EQUAL( (++iter)->second, "3" );
    BOOST_CHECK_EQUAL( (iter++)->second, "3" );
    BOOST_CHECK( iter == a.end() );
    BOOST_CHECK( a.key_begin("bar") != a.begin() );
    BOOST_CHECK( a.key_begin("foo") != a.key_begin("bar") );
    BOOST_CHECK( a.key_begin("foo") == a.key_begin("foo") );
    BOOST_CHECK( a.key_begin("foo") == a.key_cbegin("foo") );
    BOOST_CHECK( a.key_end("foo") == a.key_cend("foo") );

    const Map& b = a;

    BOOST_CHECK( b.key_begin("foo") == b.begin() );
    BOOST_CHECK( b.key_begin("foo") != b.end() );
    BOOST_CHECK( b.key_end("foo") == b.end() );
    auto iterb = b.key_begin("foo");
    BOOST_CHECK_EQUAL( (++iterb)->second, "3" );
    BOOST_CHECK_EQUAL( (iterb++)->second, "3" );
    BOOST_CHECK( iterb == b.end() );
    BOOST_CHECK( b.key_begin("bar") != b.begin() );
    BOOST_CHECK( b.key_begin("foo") != b.key_begin("bar") );
    BOOST_CHECK( b.key_begin("foo") == b.key_begin("foo") );
    BOOST_CHECK( b.key_begin("foo") == b.key_cbegin("foo") );
    BOOST_CHECK( b.key_end("foo") == b.key_cend("foo") );
}

BOOST_AUTO_TEST_CASE( test_key_range )
{
    using Map = OrderedMultimap<>;
    Map a = {{"foo", "1"}, {"bar", "2"}, {"foo", "3"}};
    const Map& b = a;

    BOOST_CHECK( a.key_range("foo").begin() == a.key_begin("foo") );
    BOOST_CHECK( a.key_range("foo").end() == a.key_end("foo") );
    BOOST_CHECK( a.key_crange("foo").begin() == a.key_cbegin("foo") );
    BOOST_CHECK( a.key_crange("foo").end() == a.key_cend("foo") );
    BOOST_CHECK( a.key_crange("foo").begin() == b.key_range("foo").begin() );
    BOOST_CHECK( a.key_crange("foo").end() == b.key_range("foo").end() );
}
