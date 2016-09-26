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
#ifndef GEO_LINE_HPP
#define GEO_LINE_HPP

#include "point.hpp"

namespace melanolib {
namespace geo {

/**
 * \brief A line segment between two points
 */
template<class Scalar, class Comparator = math::compare_equals<Scalar>>
    struct Line
{
    Point<Scalar, Comparator> p1;
    Point<Scalar, Comparator> p2;

    constexpr Line() {}
    Line(const Point<Scalar, Comparator>& p1, Scalar length, Scalar angle)
        : p1(p1), p2(PolarVector<Scalar>(length, angle).point()+p1) {}
    constexpr Line(const Point<Scalar, Comparator>& p1, const Point<Scalar, Comparator>& p2)
        : p1(p1), p2(p2) {}

    constexpr Scalar dx() const { return p2.x - p1.x; }
    constexpr Scalar dy() const { return p2.y - p1.y; }

    Scalar length() const { return p1.distance_to(p2); }
    Scalar angle() const { return math::atan2(dy(), dx()); }

    void set_angle(Scalar angle)
    {
        p2.x = p1.x + math::cos(angle) * length();
        p2.y = p1.y + math::sin(angle) * length();
    }

    void set_length(Scalar length)
    {
        p2.x = p1.x + math::cos(angle()) * length;
        p2.y = p1.y + math::sin(angle()) * length;
    }

    /**
     * \brief Gets relative point
     * \param factor when 0 returns p1, 1 p2 and in between is proportional
     */
    constexpr Point<Scalar, Comparator> point_at(Scalar factor) const
    {
        return p1*(1-factor) + p2*factor;
    }

    constexpr bool operator== (const Line& other) const
    {
        return p1 == other.p1 && p2 == other.p2;
    }

    constexpr bool operator!= (const Line& other) const
    {
        return p1 != other.p1 || p2 != other.p2;
    }
};

} // namespace geo
} // namespace melanolib
#endif // GEO_LINE_HPP
