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
#ifndef MELANOLIB_MATH_INTEGER_HPP
#define MELANOLIB_MATH_INTEGER_HPP

#include <cstdint>
namespace melanolib {

namespace detail {

template<std::size_t Bits, bool Sign> struct Small : Small<Bits - 1, Sign> {};
template<> struct Small< 0, true>  { using type = int_least8_t; };
template<> struct Small< 9, true>  { using type = int_least16_t; };
template<> struct Small<17, true>  { using type = int_least32_t; };
template<> struct Small<33, true>  { using type = int_least64_t; };
template<> struct Small<65, true>  { using type = void; };
template<> struct Small< 0, false> { using type = uint_least8_t; };
template<> struct Small< 9, false> { using type = uint_least16_t; };
template<> struct Small<17, false> { using type = uint_least32_t; };
template<> struct Small<33, false> { using type = uint_least64_t; };
template<> struct Small<65, false> { using type = void; };

template<std::size_t Bits, bool Sign> struct Fast : Fast<Bits - 1, Sign> {};
template<> struct Fast< 0, true>  { using type = int_fast8_t; };
template<> struct Fast< 9, true>  { using type = int_fast16_t; };
template<> struct Fast<17, true>  { using type = int_fast32_t; };
template<> struct Fast<33, true>  { using type = int_fast64_t; };
template<> struct Fast<65, true>  { using type = void; };
template<> struct Fast< 0, false> { using type = uint_fast8_t; };
template<> struct Fast< 9, false> { using type = uint_fast16_t; };
template<> struct Fast<17, false> { using type = uint_fast32_t; };
template<> struct Fast<33, false> { using type = uint_fast64_t; };
template<> struct Fast<65, false> { using type = void; };

} // namespace detail

template<std::size_t Bits>
    using Int = typename detail::Small<Bits, true>::type;

template<std::size_t Bits>
    using Uint = typename detail::Small<Bits, false>::type;

template<std::size_t Bits>
    using FastInt = typename detail::Fast<Bits, true>::type;

template<std::size_t Bits>
    using FastUint = typename detail::Fast<Bits, false>::type;

} // namespace melanolib
#endif // MELANOLIB_MATH_INTEGER_HPP
