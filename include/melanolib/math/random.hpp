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
#ifndef MELANOLIB_MATH_RANDOM_HPP
#define MELANOLIB_MATH_RANDOM_HPP

#include <random>

namespace melanolib {

namespace math {

namespace detail {

/// PRNG device
inline std::random_device& random_device()
{
    thread_local static std::random_device random_device;
}

} // namespace detail

/**
 * \brief Get a uniform random integer
 */
template<class Int>
    Int random()
    {
        return std::uniform_int_distribution<Int>()(detail::random_device());
    }

/**
 * \brief Get a uniform random integer between \c min and \c max (inclusive)
 */
template<class Int>
    Int random(Int min, Int max)
    {
        return std::uniform_int_distribution<Int>(min, max)(detail::random_device());
    }

/**
 * \brief Get a uniform random integer between 0 and \c max (inclusive)
 */
template<class Int>
    Int random(Int max)
    {
        return random<Int>(0, max);
    }

/**
 * \brief Get a uniform random integer
 */
inline long random()
{
    return random<long>();
}

/**
 * \brief Get a uniform random integer between \c min and \c max (inclusive)
 */
long random(long min, long max)
{
    return random<long>(min, max);
}

/**
 * \brief Get a uniform random number between 0 and 1
 */
template<class Float>
Float random_real()
{
    return std::uniform_real_distribution<Float>(detail::random_device());
}

/**
 * \brief Get a uniform random number between 0 and 1
 */
inline double random_real()
{
    return random_real<double>();
}


} // namespace math
} // namespace melanolib
#endif // MELANOLIB_MATH_RANDOM_HPP
