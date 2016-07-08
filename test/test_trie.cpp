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
#define BOOST_TEST_MODULE Test_Trie

#include <boost/test/unit_test.hpp>

#include "melanolib/string/trie.hpp"

using namespace melanolib;

BOOST_AUTO_TEST_CASE( test_trie_insert )
{
    string::Trie trie;
    BOOST_CHECK ( !trie.contains_prefix("hello") );

    trie.insert("hello");
    BOOST_CHECK ( trie.contains_prefix("hello") );
    BOOST_CHECK ( trie.contains_prefix("hell") );
    BOOST_CHECK ( trie.contains("hello") );
    BOOST_CHECK ( !trie.contains("hell") );

    trie.insert("hell");
    BOOST_CHECK ( trie.contains_prefix("hello") );
    BOOST_CHECK ( trie.contains_prefix("hell") );
    BOOST_CHECK ( trie.contains("hello") );
    BOOST_CHECK ( trie.contains("hell") );
}

BOOST_AUTO_TEST_CASE( test_trie_erase )
{
    string::Trie trie;

    trie.insert("hello");
    trie.insert("he");
    BOOST_CHECK ( trie.contains_prefix("hello") );
    BOOST_CHECK ( trie.contains_prefix("hell") );

    trie.erase("hello");
    BOOST_CHECK ( !trie.contains_prefix("hel") );
    BOOST_CHECK ( trie.contains("he") );
}

BOOST_AUTO_TEST_CASE( test_trie_initializer )
{
    string::Trie trie{"pony","princess"};

    BOOST_CHECK ( trie.contains_prefix("prince") );
    BOOST_CHECK ( !trie.contains("prince") );
    BOOST_CHECK ( trie.contains("pony") );
}

BOOST_AUTO_TEST_CASE( test_trie_prepend )
{
    string::Trie trie{"pony","princess"};

    trie.prepend(' ');
    BOOST_CHECK ( trie.contains_prefix(" prince") );
    BOOST_CHECK ( trie.contains(" pony") );
    BOOST_CHECK ( !trie.contains("pony") );
    BOOST_CHECK ( !trie.contains_prefix("prince") );

    trie.prepend("little");
    BOOST_CHECK ( !trie.contains(" pony") );
    BOOST_CHECK ( trie.contains("little pony") );

    trie.prepend("");
    BOOST_CHECK ( trie.contains("little pony") );
}

BOOST_AUTO_TEST_CASE( test_trie_data )
{
    string::Trie trie{"pony","princess"};
    trie.root().data();
    BOOST_CHECK ( std::is_void<decltype(trie.root().data())>::value );

    string::BasicTrie<std::string> string_trie;
    string_trie.insert("pony","little");
    BOOST_CHECK ( string_trie.find("pony").data() == "little" );

    auto made_trie_assoc = string::make_trie(std::unordered_map<std::string,int>{ {"foo",5}, {"bar",6} });
    BOOST_CHECK ( made_trie_assoc.find("foo").data() == 5 );

    auto made_trie = string::make_trie(std::vector<std::string>{"foo","bar"});
    BOOST_CHECK ( made_trie.contains("foo") );
}

BOOST_AUTO_TEST_CASE( test_trie_iterator )
{
    string::Trie trie{"pretty","pony","princess","priceless"};
    auto iter = trie.root();
    BOOST_CHECK ( iter.root() );
    BOOST_CHECK ( iter.can_move_down('p') );
    BOOST_CHECK ( !iter.can_move_down('q') );
    BOOST_CHECK ( iter.valid() );
    BOOST_CHECK ( iter.depth() == 0 );
    iter.move_down('p');
    iter.move_down('r');
    BOOST_CHECK ( iter.can_move_down('e') );
    BOOST_CHECK ( iter.can_move_down('i') );
    BOOST_CHECK ( !iter.can_move_down('o') );
    BOOST_CHECK ( iter.valid() );
    BOOST_CHECK ( !iter.root() );
    BOOST_CHECK ( iter.depth() == 2 );
    iter.move_up();
    BOOST_CHECK ( iter.depth() == 1 );
    iter.move_down('o');
    BOOST_CHECK ( iter.can_move_down('n') );
    BOOST_CHECK ( iter.valid() );
    BOOST_CHECK ( !iter.root() );
    BOOST_CHECK ( iter.depth() == 2 );
    iter.move_down('n');
    iter.move_down('y');
    BOOST_CHECK ( !iter.can_move_down('.') );
    BOOST_CHECK ( iter.valid() );
    BOOST_CHECK ( !iter.root() );
    BOOST_CHECK ( iter.depth() == 4 );
    iter.move_down('.');
    BOOST_CHECK ( !iter.valid() );
    BOOST_CHECK ( iter.depth() == 0 );

    BOOST_CHECK ( string::StringTrie::iterator().data() == "" );

}

BOOST_AUTO_TEST_CASE( test_trie_add )
{
    string::StringTrie trie1;
    trie1.insert("pony", "awesome");
    trie1.insert("princess", "twilight");

    string::StringTrie trie2;
    trie2.insert("pony", "little");
    trie2.insert("pretty", "good");
    trie2.insert("fun", "pink");

    trie1 += trie2;

    BOOST_CHECK ( trie1.find("pony").data() == "awesome" );
    BOOST_CHECK ( trie1.find("princess").data() == "twilight" );
    BOOST_CHECK ( trie1.find("pretty").data() == "good" );
    BOOST_CHECK ( trie1.find("fun").data() == "pink" );
    BOOST_CHECK ( trie1.contains("pony") );
    BOOST_CHECK ( trie1.contains("princess") );
    BOOST_CHECK ( trie1.contains("pretty") );
    BOOST_CHECK ( trie1.contains("fun") );

    BOOST_CHECK ( trie2.find("pony").data() == "little" );
    BOOST_CHECK ( trie2.find("pretty").data() == "good" );
    BOOST_CHECK ( trie2.find("fun").data() == "pink" );
    BOOST_CHECK ( trie2.contains("pony") );
    BOOST_CHECK ( !trie2.contains("princess") );
    BOOST_CHECK ( trie2.contains("pretty") );
    BOOST_CHECK ( trie2.contains("fun") );


    string::StringTrie trie3;
    trie3.insert("pony", "awesome");
    trie3.insert("princess", "twilight");
    trie3 += trie3;

    BOOST_CHECK ( trie3.find("pony").data() == "awesome" );
    BOOST_CHECK ( trie3.find("princess").data() == "twilight" );
    BOOST_CHECK ( trie3.contains("pony") );
    BOOST_CHECK ( trie3.contains("princess") );

    trie3 += std::move(trie2);

    BOOST_CHECK ( trie3.find("pony").data() == "awesome" );
    BOOST_CHECK ( trie3.find("princess").data() == "twilight" );
    BOOST_CHECK ( trie3.find("pretty").data() == "good" );
    BOOST_CHECK ( trie3.find("fun").data() == "pink" );
    BOOST_CHECK ( trie3.contains("pony") );
    BOOST_CHECK ( trie3.contains("princess") );
    BOOST_CHECK ( trie3.contains("pretty") );
    BOOST_CHECK ( trie3.contains("fun") );

}
