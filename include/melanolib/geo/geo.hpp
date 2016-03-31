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
#ifndef MELANOLIB_GEO_HPP
#define MELANOLIB_GEO_HPP

#include "circle.hpp"
#include "line.hpp"
#include "rectangle.hpp"

namespace melanolib {
namespace geo {

namespace geo_float {
    using Scalar = float;
    using Point = melanolib::geo::Point<Scalar>;
    using Size = melanolib::geo::Size<Scalar>;
    using PolarVector = melanolib::geo::PolarVector<Scalar>;
    using Line = melanolib::geo::Line<Scalar>;
    using Rectangle = melanolib::geo::Rectangle<Scalar>;
    using Circle = melanolib::geo::Circle<Scalar>;
    using melanolib::geo::distance;
} // namespace geo_float

namespace geo_double {
    using Scalar = double;
    using Point = melanolib::geo::Point<Scalar>;
    using Size = melanolib::geo::Size<Scalar>;
    using PolarVector = melanolib::geo::PolarVector<Scalar>;
    using Line = melanolib::geo::Line<Scalar>;
    using Rectangle = melanolib::geo::Rectangle<Scalar>;
    using Circle = melanolib::geo::Circle<Scalar>;
    using melanolib::geo::distance;
} // namespace geo_float

namespace geo_int {
    using Scalar = int;
    using Point = melanolib::geo::Point<Scalar>;
    using Size = melanolib::geo::Size<Scalar>;
    using PolarVector = melanolib::geo::PolarVector<Scalar>;
    using Line = melanolib::geo::Line<Scalar>;
    using Rectangle = melanolib::geo::Rectangle<Scalar>;
    using Circle = melanolib::geo::Circle<Scalar>;
    using melanolib::geo::distance;
} // namespace geo_float

} // namespace geo
} // namespace melanolib

#endif // MELANOLIB_GEO_HPP
