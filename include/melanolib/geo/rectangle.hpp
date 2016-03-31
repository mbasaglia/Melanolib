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
#ifndef GEO_RECTANGLE_HPP
#define GEO_RECTANGLE_HPP

#include <algorithm>
#include "point.hpp"

namespace melanolib {
namespace geo {

/**
 * \brief An axis-aligned rectangle defined by its position (top left corner) and size
 */
template<class Scalar, class Comparator = math::compare_equals<Scalar>>
    struct Rectangle
{
    Scalar x = 0;
    Scalar y = 0;
    Scalar width = 0; ///< Width \note If not positive, renders the rectangle invalid
    Scalar height = 0;///< Height \note If not positive, renders the rectangle invalid

    /**
     * \brief Construct an empty rectangle
     */
    constexpr Rectangle() {}

    constexpr Rectangle(Scalar x, Scalar y, Scalar width, Scalar height )
        : x(x),y(y),width(width),height(height) {}

    constexpr Rectangle( const Point<Scalar, Comparator>& pos, const Size<Scalar, Comparator>& size )
        : x(pos.x), y(pos.y), width(size.width), height(size.height) {}

    constexpr Rectangle( const Point<Scalar, Comparator>& top_left, const Point<Scalar, Comparator>& bottom_right )
        : x(top_left.x), y(top_left.y), width(bottom_right.x-top_left.x), height(bottom_right.y-top_left.y) {}

    constexpr Scalar top() const { return y; }
    constexpr Scalar left() const { return x; }
    constexpr Scalar right() const { return x+width; }
    constexpr Scalar bottom() const { return y+height; }

    constexpr Point<Scalar, Comparator> top_left() const { return Point<Scalar, Comparator>(left(),top()); }
    constexpr Point<Scalar, Comparator> bottom_right() const { return Point<Scalar, Comparator>(right(),bottom()); }
    constexpr Point<Scalar, Comparator> top_right() const { return Point<Scalar, Comparator>(right(),top()); }
    constexpr Point<Scalar, Comparator> bottom_left() const { return Point<Scalar, Comparator>(left(),bottom()); }
    constexpr Point<Scalar, Comparator> center() const { return Point<Scalar, Comparator>(x+width/2,y+height/2); }

    constexpr Scalar area() const { return width*height; }
    constexpr Size<Scalar, Comparator> size() const { return Size<Scalar, Comparator>(width,height); }

    /**
     * \brief Get whether a rectangle contains the given point
     */
    constexpr bool contains(const Point<Scalar, Comparator>& p) const
    {
        return p.x >= x && p.x <= x+width && p.y >= y && p.y <= y+height;
    }
    /**
     * \brief Get whether a rectangle contains the given point
     */
    constexpr bool contains(Scalar x, Scalar y) const
    {
        return contains(Point<Scalar, Comparator>(x,y));
    }

    /**
     * \brief Whether the intersection between this and \c rect is not empty
     */
    constexpr bool intersects(const Rectangle& rect) const
    {
        return x < rect.right() && right() > rect.x && y < rect.bottom() && bottom() > rect.y;
    }

    /**
     * \brief Move the rectangle by the given offset
     */
    SUPER_CONSTEXPR void translate(const Point<Scalar, Comparator>& offset)
    {
        x += offset.x;
        y += offset.y;
    }
    /**
     * \brief Move the rectangle by the given offset
     */
    SUPER_CONSTEXPR void translate(Scalar dx, Scalar dy)
    {
        translate(Point<Scalar, Comparator>(dx,dy));
    }

    /**
     * \brief Get a rectangle moved by the given amount
     */
    constexpr Rectangle translated(const Point<Scalar, Comparator>& offset) const
    {
        return Rectangle(top_left()+offset,size());
    }
    /**
     * \brief Get a rectangle moved by the given amount
     */
    constexpr Rectangle translated(Scalar dx, Scalar dy) const
    {
        return translated(Point<Scalar, Comparator>(dx,dy));
    }

    /**
     * \brief Get the rectangle corresponding to the overlapping area between the two rectangles
     * \return The overlapping area or an invalid rectangle
     */
    SUPER_CONSTEXPR Rectangle intersection(const Rectangle&rect) const
    {
        if ( !is_valid() || ! rect.is_valid() )
            return Rectangle();
        return Rectangle (
                Point<Scalar, Comparator>(
                    math::max(x, rect.x),
                    math::max(y, rect.y)
                ),
                Point<Scalar, Comparator>(
                    math::min(x+width, rect.x+rect.width),
                    math::min(y+height, rect.y+rect.height)
                )
            );
    }

    /**
     * \brief Get a rectangle large enough to contain both rectangles
     * \note If any rectangle is invalid, the other one is returned
     */
    Rectangle united(const Rectangle&rect) const
    {
        if ( !rect.is_valid() )
            return *this;
        if ( !is_valid() )
            return rect;
        return Rectangle(
                Point<Scalar, Comparator>(
                    std::min(x, rect.x),
                    std::min(y, rect.y)
                ),
                Point<Scalar, Comparator>(
                    std::max(x + width, rect.x + rect.width),
                    std::max(y + height, rect.y + rect.height)
                 )
            );
    }

    /**
     * \brief Grow this rectangle enough to contain rect
     */
    void unite(const Rectangle&rect)
    {
        *this = united(rect);
    }

    /**
     * \brief Unite
     */
    Rectangle& operator|= ( const Rectangle& rect )
    {
        return *this = united(rect);
    }

    /**
     * \brief Unite
     */
    Rectangle operator| ( const Rectangle& rect )
    {
        return united(rect);
    }

    /**
     * \brief Intersect
     */
    Rectangle& operator&= ( const Rectangle& rect )
    {
        return *this = intersection(rect);
    }

    /**
     * \brief Intersect
     */
    Rectangle operator& ( const Rectangle& rect )
    {
        return intersection(rect);
    }

    /**
     * \brief Whether the rectangle is valid (ie both \c width and \c height are positive)
     */
    bool is_valid() const
    {
        return width > 0 && height > 0;
    }

    bool operator==(const Rectangle& rect) const
    {
        return x == rect.x && y == rect.y && width == rect.width && height == rect.height;
    }
    bool operator!=(const Rectangle& rect) const
    {
        return !(*this == rect);
    }

    /**
     * \brief Get the point within the rectangle that's closest to the given point
     */
    Point<Scalar, Comparator> nearest(const Point<Scalar, Comparator>& p) const
    {
        return Point<Scalar, Comparator>(
            p.x > right()  ? right()  : ( p.x < left()? left(): p.x ),
            p.y > bottom() ? bottom() : ( p.y < top() ? top() : p.y )
        );
    }

    /**
     * \brief Add (or subtract) the given margin from each edge
     */
    void expand(Scalar margin)
    {
        x -= margin;
        y -= margin;
        width += 2*margin;
        height += 2*margin;
    }

    /**
     * \brief Get a rectangle larger than the current one by the given margin
     */
    Rectangle expanded(Scalar margin) const
    {
        return Rectangle(x-margin,y-margin,width+2*margin,height+2*margin);
    }
};

} // namespace geo
} // namespace melanolib
#endif // GEO_RECTANGLE_HPP
