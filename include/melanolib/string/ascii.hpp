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
#ifndef MELANOLIB_STRING_ASCII_HPP
#define MELANOLIB_STRING_ASCII_HPP

#include <cstdint>
#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {
namespace string {
/**
 * \brief Namespace for queries and operations on ASCII-encoded characters
 */
namespace ascii {

/**
 * \brief Whether \p c is an ASCII character [0-127]
 */
inline constexpr bool is_ascii(char c)
{
    return c >= 0 && c <= 127;
}

/**
 * \brief Whether \p c is a control character
 *
 * Equivalent to std::iscntrl
 */
inline constexpr bool is_cntrl(char c)
{
    return c < 0x20 || c == 0x7f;
}

/**
 * \brief Whether \p c is a printable character
 *
 * Equivalent to std::isprint
 */
inline constexpr bool is_print(char c)
{
    return c >= 0x20 && c < 0x7f;
}

/**
 * \brief Whether \p c is a whitspace character
 *
 * Equivalent to std::isspace
 */
inline constexpr bool is_space(char c)
{
    return (c >= 0x09 && c <= 0x0d) || c == ' ';
}

/**
 * \brief Whether \p c is a blank character (Space used to separate words)
 *
 * Equivalent to std::isblank
 */
inline constexpr bool is_blank(char c)
{
    return c == ' ' || c == '\t';
}

/**
 * \brief Whether \p c is a space that does not mark the end of a line
 *
 * Same as is_space() but returns \b false for \r and \n
 */
inline constexpr bool is_space_noline(char c)
{
    return c == '\t' || c == '\v' || c == '\f' || c == ' ';
}

/**
 * \brief Whether \p c has a graphical representation
 *
 * Equivalent to std::isgraph
 */
inline constexpr bool is_graph(char c)
{
    return c > 0x20 && c < 0x7f;
}

/**
 * \brief Whether \p c is a punctuation character
 *
 * Equivalent to std::ispunct
 */
inline constexpr bool is_punct(char c)
{
    return (c >= 0x21 && c <= 0x2f) || (c >= 0x3a && c <= 0x40) ||
           (c >= 0x5b && c <= 0x60) || (c >= 0x7b && c <= 0x7e);
}

/**
 * \brief Whether \p c is an uppercase alphabetic character
 *
 * Equivalent to std::isupper
 */
inline constexpr bool is_upper(char c)
{
    return (c >= 'A' && c <= 'Z');
}

/**
 * \brief Whether \p c is a lower case alphabetic character
 *
 * Equivalent to std::islower
 */
inline constexpr bool is_lower(char c)
{
    return (c >= 'a' && c <= 'z');
}

/**
 * \brief Whether \p c is an alphabetic character
 *
 * Equivalent to std::isalpha
 */
inline constexpr bool is_alpha(char c)
{
    return is_upper(c) || is_lower(c);
}

/**
 * \brief Whether \p c is a decimal digit
 *
 * Equivalent to std::isdigit
 */
inline constexpr bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

/**
 * \brief Whether \p c is alphabetic or numerical
 *
 * Equivalent to std::isalnum
 */
inline constexpr bool is_alnum(char c)
{
    return is_alpha(c) || is_digit(c);
}

/**
 * \brief Whether \p c is a hexadecimal digit
 *
 * Equivalent to std::isxdigit
 */
inline constexpr bool is_xdigit(char c)
{
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/**
 * \brief Returns an (uppercase) hex digit from a 4-bit integer
 * \pre \p digit & 0xF0 == 0
 */
inline constexpr char hex_digit(uint8_t digit)
{
    return digit > 9 ? digit - 10 + 'A' : digit + '0';
}

/**
 * \brief Returns a (4-bit) integer representing the value of the hexadecimal digit
 * \pre is_xdigit(\p ch)
 */
inline constexpr int get_hex(char ch)
{
    return ch > '9' ?
        ( ch >= 'a' ? ch - 'a' + 10 : ch - 'A' + 10 )
        : ch - '0'
    ;
}

/**
 * \brief Transforms lower-case characters to upper case
 *
 * Equivalent to std::toupper
 */
inline SUPER_CONSTEXPR char to_upper(char c)
{
    if ( is_lower(c) )
        return c - 'a' + 'A';
    return c;
}

/**
 * \brief Transforms upper-case characters to lower case
 *
 * Equivalent to std::tolower
 */
inline SUPER_CONSTEXPR char to_lower(char c)
{
    if ( is_upper(c) )
        return c - 'A' + 'a';
    return c;
}

} // namespace ascii
} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_ASCII_HPP
