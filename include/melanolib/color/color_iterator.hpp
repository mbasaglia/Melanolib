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
#ifndef MELANOLIB_COLOR_ITERATOR_HPP
#define MELANOLIB_COLOR_ITERATOR_HPP

#include <iterator>

namespace melanolib {
namespace color {

template<class Range, class Repr>
class ColorIterator
{
public:
    using size_type = std::size_t;
    using value_type = Repr;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type =  std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    constexpr ColorIterator() : ColorIterator(nullptr, 0) {}

    constexpr value_type operator*() const
    {
        return color();
    }

    constexpr ColorIterator& operator++()
    {
        if ( offset < range->size() )
            offset += 1;
        return *this;
    }

    constexpr ColorIterator operator++(int)
    {
        auto iter = *this;
        ++*this;
        return iter;
    }

    constexpr ColorIterator& operator--()
    {
        if ( offset > 0 )
            offset -= 1;
        return *this;
    }

    constexpr ColorIterator operator--(int)
    {
        auto iter = *this;
        --*this;
        return iter;
    }

    constexpr ColorIterator& operator+=(difference_type off)
    {
        if ( off > 0 )
        {
            if ( offset + off > range->size() )
                offset = range->size();
            else
                offset += off;
        }
        else
        {
            if ( size_type(-off) < offset )
                offset += off;
            else
                offset = 0;
        }

        return *this;
    }

    constexpr ColorIterator operator+(difference_type off) const
    {
        auto iter = *this;
        return iter += off;
    }

    friend constexpr ColorIterator operator+(difference_type off, ColorIterator iter)
    {
        return iter += off;
    }

    constexpr ColorIterator& operator-=(difference_type off)
    {
        return *this += -off;
    }

    constexpr ColorIterator operator-(difference_type off) const
    {
        auto iter = *this;
        return iter -= off;
    }

    constexpr difference_type operator-(const ColorIterator& oth) const
    {
        return offset - oth.offset;
    }

    constexpr value_type operator[](difference_type off) const
    {
        return *(*this + off);
    }

    constexpr bool operator==(const ColorIterator& oth) const
    {
        return (!valid() && !oth.valid()) ||
                (range == oth.range && offset == oth.offset);
    }

    constexpr bool operator!=(const ColorIterator& oth) const
    {
        return !(*this == oth);
    }

    constexpr bool valid() const
    {
        return range && offset <= range->size();
    }

    constexpr bool operator<(const ColorIterator& oth) const
    {
        return offset < oth.offset;
    }

    constexpr bool operator<=(const ColorIterator& oth) const
    {
        return offset <= oth.offset;
    }

    constexpr bool operator>(const ColorIterator& oth) const
    {
        return offset > oth.offset;
    }

    constexpr bool operator>=(const ColorIterator& oth) const
    {
        return offset >= oth.offset;
    }

    static constexpr ColorIterator begin(const Range& range)
    {
        return ColorIterator(&range, 0);
    }

    static constexpr ColorIterator end(const Range& range)
    {
        return ColorIterator(&range, range.size());
    }

private:
    constexpr ColorIterator(const Range* range, size_type offset)
        : range(range), offset(offset)
    {}

    constexpr value_type color() const
    {
        return range->color(double(offset) / (range->size() - 1));
    }

    const Range* range;
    size_type offset;
};

} // namespace color
} // namespace melanolib
#endif // MELANOLIB_COLOR_ITERATOR_HPP
