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
#ifndef GEO_POINT_HPP
#define GEO_POINT_HPP

#include "melanolib/math/math.hpp"
#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {

/**
 * \brief Namespace for geometric objects and operations
 */
namespace geo {

/**
 * \brief A point in a 2D space
 */
template<class Scalar, class Comparator = math::compare_equals<Scalar>>
    struct Point
{
    Scalar x = 0; ///< X coordinate
    Scalar y = 0; ///< Y coordinate

    constexpr Point(Scalar x, Scalar y) : x(x), y(y) {}
    constexpr Point() {}

    SUPER_CONSTEXPR Point& operator+= ( const Point& p )
    {
        x += p.x;
        y += p.y;
        return *this;
    }
    SUPER_CONSTEXPR Point& operator-= ( const Point& p )
    {
        x -= p.x;
        y -= p.y;
        return *this;
    }
    SUPER_CONSTEXPR Point& operator*= ( Scalar factor )
    {
        x *= factor;
        y *= factor;
        return *this;
    }
    SUPER_CONSTEXPR Point& operator/= ( Scalar factor )
    {
        x /= factor;
        y /= factor;
        return *this;
    }

    constexpr Point operator- () const
    {
        return Point(-x, -y);
    }
    constexpr Point operator+ () const
    {
        return *this;
    }

    constexpr Point operator+ (const Point&p) const
    {
        return Point(p.x + x, p.y + y);
    }
    constexpr Point operator- (const Point&p) const
    {
        return Point(x-p.x, y-p.y);
    }
    constexpr Point operator* (Scalar factor) const
    {
        return Point(x * factor, y * factor);
    }
    constexpr Point operator/ (Scalar factor) const
    {
        return Point(x/  factor, y / factor);
    }

    constexpr bool operator== ( const Point& p ) const
    {
        return Comparator()(x, p.x) && Comparator()(y, p.y);
    }

    constexpr bool operator!= ( const Point& p ) const
    {
        return !Comparator()(x, p.x) || !Comparator()(y, p.y);
    }

    /**
     * \brief Distance from the origin
     */
    Scalar magnitude () const
    {
        return melanolib::math::sqrt(x*x + y*y);
    }

    /**
     * \brief Distance to another Point
     */
    Scalar distance_to (const Point& o) const
    {
        return (*this-o).magnitude();
    }
};

template<class Scalar1, class Scalar2, class Comparator>
    constexpr Point<Scalar2> operator* (Scalar1 factor, const Point<Scalar2, Comparator>& p)
{
    return Point<Scalar2>(p.x * factor, p.y * factor);
}

/**
 * \brief 2-norm distance between two points
 */
template<class Scalar, class Comparator>
    inline Scalar distance (const Point<Scalar, Comparator>& a, const Point<Scalar, Comparator>& b)
{
    return a.distance_to(b);
}


/**
 * \brief A size ( width/height )
 */
template<class Scalar, class Comparator = math::compare_equals<Scalar>>
    struct Size
{
    Scalar width  = 0;
    Scalar height = 0;

    constexpr Size ( Scalar width, Scalar height ) : width(width), height(height) {}
    constexpr Size(){}

    constexpr bool operator== (const Size& other) const
    {
        return Comparator()(width, other.width) && Comparator()(height, other.height);
    }
    constexpr bool operator!= (const Size& other) const
    {
        return !Comparator()(width, other.width) || !Comparator()(height, other.height);
    }
};


/**
 * \brief A Point defined in polar coordinates
 */
template<class Scalar, class Comparator = math::compare_equals<Scalar>>
    struct PolarVector
{
    Scalar length = 0; ///< Length
    Scalar angle  = 0; ///< Angle in radians

    constexpr PolarVector() = default;
    constexpr PolarVector(Scalar length, Scalar angle) : length(length), angle(angle) {}
    PolarVector(Point<Scalar, Comparator> point)
        : length(point.magnitude()), angle(math::atan2(point.y, point.x)) {}

    /**
     * \brief Converts to cartesian
     */
    Point<Scalar, Comparator> point() const
    {
        return Point<Scalar, Comparator>( math::cos(angle) * length, math::sin(angle) * length );
    }

    PolarVector& operator+= (const Point<Scalar>& p)
    {
        return *this = point() + p;
    }

    constexpr bool operator== (const PolarVector& other) const
    {
        return Comparator()(length, other.length) && Comparator()(angle, other.angle);
    }

    constexpr bool operator!= (const PolarVector& other) const
    {
        return length != other.length || angle != other.angle;
    }
};

} // namespace geo
} // namespace melanolib

#endif // GEO_POINT_HPP
