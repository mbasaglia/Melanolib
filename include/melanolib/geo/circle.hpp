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
#ifndef MELANOLIB_GEO_CIRCLE_HPP
#define MELANOLIB_GEO_CIRCLE_HPP

#include "rectangle.hpp"

namespace melanolib {
namespace geo {

template<class Scalar, class Comparator = math::compare_equals<Scalar>>
    struct Circle
{
    Point<Scalar, Comparator> origin;
    Scalar radius = 0;

    Circle() {}
    Circle(Scalar x, Scalar y, Scalar radius = 0)
        : origin(x,y), radius(radius){}
    Circle(const Point<Scalar, Comparator>& origin, Scalar radius = 0)
        : origin(origin), radius(radius){}

    /**
     * \brief Whether a point is inside the circle
     */
    bool contains(const Point<Scalar, Comparator>& p) const
    {
        return origin.distance_to(p) <= math::abs(radius);
    }

    /**
     * \brief Whether two circles intersect
     */
    bool intersects(const Circle& o) const
    {
        return origin.distance_to(o.origin) <= math::abs(radius) + math::abs(o.radius);
    }

    /**
     * \brief Whether a rectangle is fully contained within the circle
     */
    bool contains(const Rectangle<Scalar, Comparator>& rect) const
    {
        return rect.is_valid() &&
               contains(rect.top_left())    &&
               contains(rect.top_right())   &&
               contains(rect.bottom_right())&&
               contains(rect.bottom_left()) ;
    }

    /**
     * \brief Whether a rectangle intersects the circle
     */
    bool intersects(const Rectangle<Scalar, Comparator>& rect) const
    {
        return rect.nearest(origin).distance_to(origin) < math::abs(radius);
    }
};

} // namespace geo
} // namespace melanolib
#endif // MELANOLIB_GEO_CIRCLE_HPP
