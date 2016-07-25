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
#ifndef MELANOLIB_MATH_HPP
#define MELANOLIB_MATH_HPP

#include <algorithm>
#include <type_traits>
#include <cmath>

namespace melanolib {
namespace math {

constexpr double pi = 3.1415926535897932384626433832795;
constexpr double tau = 6.283185307179586476925286766559;
constexpr double e = 2.718281828459045235360287471352662;

using std::fmod;

using std::sqrt;
using std::exp;
using std::pow;
using std::log;
using std::log2;

using std::sin;
using std::cos;
using std::tan;
using std::asin;
using std::acos;
using std::atan;
using std::atan2;

using std::sinh;
using std::cosh;
using std::tanh;
using std::asinh;
using std::acosh;
using std::atanh;

/**
 * \brief Truncates a number (rounds towards zero)
 * \tparam Return   Return type (Must be an integral type)
 * \tparam Argument Argument type (Must be a floating point type)
 */
template<class Return=int, class Argument=double>
    constexpr Return truncate(Argument x)
    {
        return Return(x);
    }

/**
 * \brief Rounds a number towards the closest integer
 * \tparam Return   Return type (Must be an integral type)
 * \tparam Argument Argument type (Must be a floating point type)
 */
template<class Return=int, class Argument=double>
    constexpr Return round(Argument x)
    {
        return truncate<Return>(x + Argument(x < 0 ? -0.5 : 0.5));
    }

/**
 * \brief Get the fractional part of a floating-point number
 * \tparam Argument Argument type (Must be a floating point type)
 *
 * For negative numbers, it returns a negative value
 * (eg: fractional(-3.4) == -0.4)
 */
template<class Argument>
    constexpr auto fractional(Argument x)
    {
        return x - truncate<long long>(x);
    }

/**
 * \brief Get the distance of a floating-point number from the previous integer
 * \tparam Argument Argument type (Must be a floating point type)
 *
 * For negative numbers, it returns a positive value
 * (eg: fractional(-3.4) == 0.6)
 */
template<class Argument>
    constexpr auto positive_fractional(Argument x)
    {
        return x < 0 && fractional(x) < 0 ? 1 + fractional(x) : fractional(x);
    }

/**
 * \brief Rounds a number towards negative infinitive
 * \tparam Return   Return type (Must be an integral type)
 * \tparam Argument Argument type (Must be a floating point type)
 */
template<class Return=int, class Argument=double>
    constexpr Return floor(Argument x)
    {
        return truncate<Return>(x < 0 ? x - positive_fractional(x) : x);
    }

/**
 * \brief Rounds a number towards negative infinitive
 * \tparam Return   Return type (Must be an integral type)
 * \tparam Argument Argument type (Must be a floating point type)
 */
template<class Return=int, class Argument=double>
    constexpr Return ceil(Argument x)
    {
        return truncate<Return>(x < 0 || fractional(x) == 0 ? x : x + 1 - fractional(x));
    }

/**
 * \brief (Stable) maximum between two values
 */
template<class T, class U>
    inline constexpr T max(T&& a, U&& b)
    {
        return a < b ? b : a;
    }

/**
 * \brief (Stable) maximum among several values
 */
template<class T, class...Ts>
    inline constexpr T max(T&& a, Ts&&... b)
    {
        return max(std::forward<T>(a), max(std::forward<Ts>(b)...));
    }

/**
 * \brief (Stable) minimum between two values
 */
template<class T, class U>
    inline constexpr T min(T&& a, U&& b)
    {
        return !(b < a) ? a : b;
    }

/**
 * \brief (Stable) minimum among several values
 */
template<class T, class...Ts>
    inline constexpr T min(T&& a,  Ts&&... b)
    {
        return min(std::forward<T>(a), min(std::forward<Ts>(b)...));
    }

/**
 * \brief Absolute value
 */
template<class T>
    inline constexpr T abs(T x)
    {
        return x < 0 ? -x : x;
    }

/**
 * \brief Normalize a value
 * \pre  value in [min, max] && min < max
 * \post value in [0, 1]
 */
template<class Real>
    inline constexpr Real normalize(Real value, Real min, Real max)
{
    return (value - min) / (max - min);
}

/**
 * \brief Denormalize a value
 * \pre  value in [0, 1] && min < max
 * \post value in [min, max]
 */
template<class Real>
    inline constexpr Real denormalize(Real value, Real min, Real max)
{
    return value * (max - min) + min;
}

/**
 * \brief Clamp a value inside a range
 * \tparam Argument Argument type (Must be a floating point type)
 * \param min_value Minimum allowed value
 * \param value     Variable to be bounded
 * \param max_value Maximum allowed value
 * \pre min_value < max_value
 * \post value in [min_value, max_value]
 */
template<class Argument>
    constexpr auto bound(Argument&& min_value, Argument&& value, Argument&& max_value)
    {
        return max(std::forward<Argument>(min_value),
            min(std::forward<Argument>(value), std::forward<Argument>(max_value))
        );
    }

template<class Argument, class Arg2, class = std::enable_if_t<!std::is_same<Argument, Arg2>::value>>
    constexpr auto bound(Arg2&& min_value, Argument&& value, Arg2&& max_value)
    {
        using Common = std::common_type_t<Arg2, Argument>;
        return bound<Common>(
            Common(std::forward<Arg2>(min_value)),
            Common(std::forward<Argument>(value)),
            Common(std::forward<Arg2>(max_value))
        );
    }

/**
 * \brief Compares two floating point values
 * \returns \c true if their values can be considered equal
 */
constexpr inline bool fuzzy_compare(double a, double b, double max_error = 0.001)
{
    return (abs(a - b) / (b == 0 ? 1 : b)) < max_error;
}

template<class T>
    constexpr T linear_interpolation(const T& a, const T& b, double factor)
{
    return a * (1 - factor) + b * factor;
}



namespace detail {


template<class T>
    struct fuzzy_compare_equals
    {
        constexpr bool operator() (T a, T b) const
        {
            return fuzzy_compare(a, b);
        }
    };

} // namespace detail

template<class T>
    struct compare_equals : std::conditional_t<
        std::is_floating_point<T>::value,
        detail::fuzzy_compare_equals<T>,
        std::equal_to<T>
    >
    {};

} // namespace math
} // namespace melanolib

#endif // MELANOLIB_MATH_HPP
