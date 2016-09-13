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
 *
 */

#define BOOST_TEST_MODULE Test_IdPool

#include <boost/test/unit_test.hpp>

#include "melanolib/data_structures/id_pool.hpp"

using IdPool = melanolib::BasicIdPool<std::size_t, 10>;

BOOST_AUTO_TEST_CASE( test_simple_get_sequence )
{
    IdPool pool;
    BOOST_CHECK_EQUAL(pool.get_id(), 1);
    BOOST_CHECK_EQUAL(pool.get_id(), 2);
    BOOST_CHECK_EQUAL(pool.get_id(), 3);
    BOOST_CHECK_EQUAL(pool.get_id(), 4);
    BOOST_CHECK_EQUAL(pool.get_id(), 5);
    BOOST_CHECK_EQUAL(pool.get_id(), 6);
    BOOST_CHECK_EQUAL(pool.get_id(), 7);
    BOOST_CHECK_EQUAL(pool.get_id(), 8);
    BOOST_CHECK_EQUAL(pool.get_id(), 9);
    BOOST_CHECK_EQUAL(pool.get_id(), 10);
    BOOST_CHECK_EQUAL(pool.get_id(), 0);
    BOOST_CHECK_EQUAL(pool.get_id(), 0);
    BOOST_CHECK_EQUAL(pool.get_id(), 0);
}

BOOST_AUTO_TEST_CASE( test_get_merge )
{
    IdPool pool;
    pool.mark_id(3);
    BOOST_CHECK_EQUAL(pool.get_id(), 1);
    BOOST_CHECK_EQUAL(pool.get_id(), 2);
    BOOST_CHECK_EQUAL(pool.get_id(), 4);
    BOOST_CHECK_EQUAL(pool.get_id(), 5);
}

BOOST_AUTO_TEST_CASE( test_simple_mark )
{
    IdPool pool;
    pool.mark_id(1);
    pool.mark_id(2);
    pool.mark_id(3);
    BOOST_CHECK_EQUAL(pool.get_id(), 4);
    BOOST_CHECK_EQUAL(pool.get_id(), 5);
}

BOOST_AUTO_TEST_CASE( test_mark_merge )
{
    IdPool pool;
    pool.mark_id(3);
    pool.mark_id(5);
    pool.mark_id(4);
    BOOST_CHECK_EQUAL(pool.get_id(), 1);
    BOOST_CHECK_EQUAL(pool.get_id(), 2);
    BOOST_CHECK_EQUAL(pool.get_id(), 6);
    BOOST_CHECK_EQUAL(pool.get_id(), 7);
}

BOOST_AUTO_TEST_CASE( test_mark_back )
{
    IdPool pool;
    pool.mark_id(5);
    pool.mark_id(4);
    pool.mark_id(3);
    BOOST_CHECK_EQUAL(pool.get_id(), 1);
    BOOST_CHECK_EQUAL(pool.get_id(), 2);
    BOOST_CHECK_EQUAL(pool.get_id(), 6);
    BOOST_CHECK_EQUAL(pool.get_id(), 7);
}

BOOST_AUTO_TEST_CASE( test_mark_existing )
{
    IdPool pool;
    pool.mark_id(3);
    pool.mark_id(4);
    pool.mark_id(5);
    pool.mark_id(4);
    BOOST_CHECK_EQUAL(pool.get_id(), 1);
    BOOST_CHECK_EQUAL(pool.get_id(), 2);
    BOOST_CHECK_EQUAL(pool.get_id(), 6);
    BOOST_CHECK_EQUAL(pool.get_id(), 7);
}

BOOST_AUTO_TEST_CASE( test_mark_scatter )
{
    IdPool pool;
    pool.mark_id(3);
    pool.mark_id(7);
    pool.mark_id(5);
    BOOST_CHECK_EQUAL(pool.get_id(), 1);
    BOOST_CHECK_EQUAL(pool.get_id(), 2);
    BOOST_CHECK_EQUAL(pool.get_id(), 4);
    BOOST_CHECK_EQUAL(pool.get_id(), 6);
    BOOST_CHECK_EQUAL(pool.get_id(), 8);
    BOOST_CHECK_EQUAL(pool.get_id(), 9);
}

