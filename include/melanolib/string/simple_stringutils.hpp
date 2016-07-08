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
#ifndef MELANOLIB_STRING_SIMPLE_STRINGUTILS_HPP
#define MELANOLIB_STRING_SIMPLE_STRINGUTILS_HPP

#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "trie.hpp"

namespace melanolib {
namespace string {

/**
 * \brief Turn a container into a string
 * \pre Container::const_iterator is a ForwardIterator
 *      Container::value_type has the stream operator
 * \note Should work on arrays just fine
 */
template<class Container>
    std::string implode(const std::string& glue, const Container& elements)
    {
        auto iter = std::begin(elements);
        auto end = std::end(elements);
        if ( iter == end )
            return "";

        std::ostringstream ss;
        while ( true )
        {
            ss << *iter;
            ++iter;
            if ( iter != end )
                ss << glue;
            else
                break;
        }

        return ss.str();
    }

/**
 * \brief Whether a string starts with the given prefix
 */
inline bool starts_with(const std::string& haystack, const std::string& prefix)
{
    auto it1 = haystack.begin();
    auto it2 = prefix.begin();
    while ( it1 != haystack.end() && it2 != prefix.end() && *it1 == *it2 )
    {
        ++it1;
        ++it2;
    }
    return it2 == prefix.end();
}

/**
 * \brief Whether a string ends with the given suffix
 */
inline bool ends_with(const std::string& haystack, const std::string& suffix)
{
    auto it1 = haystack.rbegin();
    auto it2 = suffix.rbegin();
    while ( it1 != haystack.rend() && it2 != suffix.rend() && *it1 == *it2 )
    {
        ++it1;
        ++it2;
    }
    return it2 == suffix.rend();
}

/**
 * \brief String to lower case
 */
inline std::string strtolower ( std::string string )
{
    std::transform(string.begin(),string.end(),string.begin(), (int(*)(int))std::tolower);
    return string;
}

/**
 * \brief String to upper case
 */
inline std::string strtoupper ( std::string string )
{
    std::transform(string.begin(),string.end(),string.begin(), (int(*)(int))std::toupper);
    return string;
}

/**
 * \brief Collapse all sequences of spaces to a single space character ' '
 */
inline std::string collapse_spaces ( const std::string& text )
{
    static std::regex regex_spaces("\\s+");
    return std::regex_replace(text, regex_spaces, " ");
}

/**
 * \brief Converts \c string to an unsigned integer
 * \returns The corresponding integer or \c default_value on failure
 */
inline unsigned long to_uint(const std::string& string,
                      unsigned long base = 10,
                      unsigned long default_value = 0) noexcept
try {
    return std::stoul(string,0,base);
} catch(const std::exception&) {
    return default_value;
}

/**
 * \brief Converts \c string to an integer
 * \returns The corresponding integer or \c default_value on failure
 */
inline long to_int(const std::string& string,
            unsigned long base = 10,
            long default_value = 0) noexcept
try {
    return std::stol(string,0,base);
} catch(const std::exception&) {
    return default_value;
}


/**
 * \brief Check if a string is one of a given set
 */
inline bool is_one_of(const std::string& string, const std::initializer_list<std::string>& il )
{
    return std::find(il.begin(),il.end(),string) != il.end();
}

/**
 * \brief Case-insensitive string comparison
 */
inline bool icase_equal(const std::string& a, const std::string& b) noexcept
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](char l, char r) { return std::tolower(l) == std::tolower(r); });
}

/**
 * \brief Converts a number to string padding with zeros to have at least
 *      \c digits digits
 */
template<class T>
    std::string to_string(T number,int digits=-1)
    {
        auto s = std::to_string(number);
        if ( int(s.size()) < digits )
            s = std::string(digits-s.size(),'0')+s;
        return s;
    }

/**
 * \brief Trim string based on a predicate
 */
template<class Predicate>
    std::string trimmed(std::string subject, const Predicate& predicate)
    {
        subject.erase(
            subject.begin(),
            std::find_if_not(subject.begin(), subject.end(), predicate)
        );
        subject.erase(
            std::find_if_not(subject.rbegin(), subject.rend(), predicate).base(),
            subject.end()
        );
        return subject;
    }

/**
 * \brief Trims spaces
 */
inline std::string trimmed(const std::string& subject)
{
    return trimmed(subject, (int (*)(int))std::isspace);
}

/**
 * \brief Whether \p subject contains \p c
 */
inline bool contains(const std::string& subject, char c)
{
    return subject.find(c) != std::string::npos;
}

/**
 * \brief Whether \p subject contains any of the characters in \p characters
 */
inline bool contains_any(const std::string& subject, const std::string& characters)
{
    return subject.find_first_of(characters) != std::string::npos;
}

} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_SIMPLE_STRINGUTILS_HPP
