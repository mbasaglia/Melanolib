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
namespace melanolib {

namespace math {

/**
 * \brief Get a uniform random integer
 */
long random();

/**
 * \brief Get a uniform random integer between 0 and \c max (inclusive)
 */
long random(long max);

/**
 * \brief Get a uniform random integer between \c min and \c max (inclusive)
 */
long random(long min, long max);

/**
 * \brief Get a uniform random number between 0 and 1
 */
double random_real();

} // namespace math
} // namespace melanolib
#endif // MELANOLIB_MATH_RANDOM_HPP
