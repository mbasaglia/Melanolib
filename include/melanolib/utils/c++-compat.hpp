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
#ifndef MELANOLIB_CXX_COMPAT_HPP
#define MELANOLIB_CXX_COMPAT_HPP


#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
#  define SUPER_CONSTEXPR constexpr
#else
#  define SUPER_CONSTEXPR
#endif

#ifdef __has_include
#   define MELANOLIB_HAS_INCLUDE(x) __has_include(x)
#else
#   define define MELANOLIB_HAS_INCLUDE(x) false
#endif

// optional
#if MELANOLIB_HAS_INCLUDE(<optional>) && !defined(MELANOLIB_BOOST_OPTIONAL)
#    include<optional>
    namespace melanolib {
        template<class T>
            using Optional = std::optional<T>;
    } // namespace melanolib
#  elif MELANOLIB_HAS_INCLUDE(<experimental/optional>) && !defined(MELANOLIB_BOOST_OPTIONAL)
#    include <experimental/optional>
    namespace melanolib {
        template<class T>
            using Optional = std::experimental::optional<T>;
    } // namespace melanolib
#  elif MELANOLIB_HAS_INCLUDE(<boost/optional.hpp>) || defined(MELANOLIB_BOOST_OPTIONAL)
#    include <boost/optional.hpp>
    namespace melanolib {
        template<class T>
            using Optional = boost::optional<T>;
    } // namespace melanolib
#  else
#     error "Missing <optional>"
#  endif

// any
#if MELANOLIB_HAS_INCLUDE(<any>) && !defined(MELANOLIB_BOOST_ANY)
#    include<any>
    namespace melanolib {
        using Any = std::any;
        template<class T, class Arg>
            auto any_cast(Arg&& arg) {
                return std::any_cast<T>(std::forward<Arg>(arg));
            }
    } // namespace melanolib
#  elif MELANOLIB_HAS_INCLUDE(<experimental/any>) && !defined(MELANOLIB_BOOST_ANY)
#    include <experimental/any>
    namespace melanolib {
        using Any = std::experimental::any;
        template<class T, class Arg>
            auto any_cast(Arg&& arg) {
                return std::experimental::any_cast<T>(std::forward<Arg>(arg));
            }
    } // namespace melanolib
#  elif MELANOLIB_HAS_INCLUDE(<boost/any.hpp>) || defined(MELANOLIB_BOOST_ANY)
#    include <boost/any.hpp>
    namespace melanolib {
        using Any = boost::any;
        template<class T, class Arg>
            auto any_cast(Arg&& arg) {
                return boost::any_cast<T>(std::forward<Arg>(arg));
            }
    } // namespace melanolib
#  else
#     error "Missing <any>"
#  endif

// variant
#if MELANOLIB_HAS_INCLUDE(<variant>)
#    include<variant>
    namespace melanolib {
        template<class... T>
            using Variant = std::variant<T...>;
        using std::get;
    } // namespace melanolib
#  elif MELANOLIB_HAS_INCLUDE(<boost/variant.hpp>)
#    include <boost/variant.hpp>
    namespace melanolib {
        template<class... T>
            using Variant = boost::variant<T...>;
        using boost::get;
    } // namespace melanolib
#  else
#     error "Missing <variant>"
#  endif

#include <memory>

namespace melanolib {

/**
 * \brief Just a shorter version of std::make_unique
 */
template<class T, class... Args>
auto New (Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace melanolib

#if __cplusplus <= 201402L // C++17
namespace std {

template <class Container>
    constexpr auto size(const Container& container) -> decltype(container.size())
    {
        return container.size();
    }

template <class T, std::size_t Size>
    constexpr std::size_t size(const T (&array)[Size]) noexcept
    {
        return Size;
    }
} // namespace std
#endif // C++17

#ifndef __cpp_lib_invoke
namespace std {
template<class... Args>
    decltype(auto) invoke(Args&&... args) {
        return std::__invoke(std::forward<Args>(args)...);
    }
} // namespace std
#endif // C++17 std::invoke


#endif // MELANOLIB_CXX_COMPAT_HPP
