/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2016 Mattia Basaglia
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
#ifndef MELANOLIB_ICASE_FUNCTORS_HPP
#define MELANOLIB_ICASE_FUNCTORS_HPP


#include "melanolib/string/simple_stringutils.hpp"
#include "melanolib/data_structures/hash.hpp"

namespace melanolib {

/**
 * \brief Case-insensitive string comparator class
 */
struct ICaseComparator
{
    bool operator()(const std::string& a, const std::string& b) const
    {
        return melanolib::string::icase_equal(a, b);
    }
};

/**
 * \brief Case-insensitive string hashing
 */
struct ICaseHasher
{
    std::size_t operator()(const std::string& a) const
    {
        return hash(melanolib::string::strtolower(a));
    }
};

} // namespace melanolib
#endif // MELANOLIB_ICASE_FUNCTORS_HPP
