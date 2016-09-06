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
#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {
namespace color {

template<class Container>
    struct ContainerSize
    {
        auto operator()(const Container& container) const
        {
            return std::size(container);
        }
    };

template<class Range, class SizeFunctor=ContainerSize<Range>>
class ColorIterator
{
public:
    using size_type = std::size_t;
    using value_type = typename Range::value_type;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type =  std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    constexpr ColorIterator() : ColorIterator(nullptr, 0, SizeFunctor{}) {}

    constexpr value_type operator*() const
    {
        return color();
    }

    constexpr ColorIterator& operator++()
    {
        if ( offset < size(*range) )
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
            if ( offset + off > size(*range) )
                offset = size(*range);
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
        return range && offset <= size(*range);
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

private:
    constexpr ColorIterator(const Range* range, size_type offset, SizeFunctor size)
        : range(range), offset(offset), size(size)
    {}

    constexpr value_type color() const
    {
        return range->color(double(offset) / (size(*range) - 1));
    }

    const Range* range;
    size_type offset;
    SizeFunctor size;

    template<class FRange, class FSizeFunctor>
        friend constexpr ColorIterator<FRange, FSizeFunctor>
            begin(const FRange& range, const FSizeFunctor& size);


    template<class FRange, class FSizeFunctor>
        friend constexpr ColorIterator<FRange, FSizeFunctor>
            end(const FRange& range, const FSizeFunctor& size);

};

template<class Range,class SizeFunc=ContainerSize<Range>>
    constexpr ColorIterator<Range, SizeFunc>
    begin(const Range& range, const SizeFunc& size = {})
    {
        return ColorIterator<Range, SizeFunc>(&range, 0, size);
    }

template<class Range, class SizeFunc=ContainerSize<Range>>
    constexpr ColorIterator<Range, SizeFunc>
    end(const Range& range, const SizeFunc& size = {})
    {
        return ColorIterator<Range, SizeFunc>(&range, size(range), size);
    }

} // namespace color
} // namespace melanolib
#endif // MELANOLIB_COLOR_ITERATOR_HPP
