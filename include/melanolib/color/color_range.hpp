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
#ifndef MELANOLIB_COLOR_RANGE_HPP
#define MELANOLIB_COLOR_RANGE_HPP

#include <iterator>
#include "melanolib/color/color.hpp"

namespace melanolib {
namespace color {

template<class Repr=Color>
    class BasicColorRange
{
public:
    using size_type = std::size_t;
    class iterator
    {
    public:
        using value_type = Repr;
        using reference = const value_type&;
        using pointer = const value_type*;
        using difference_type =  std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        constexpr iterator() : iterator(nullptr, 0) {}

        constexpr value_type operator*() const
        {
            return color();
        }

        constexpr iterator& operator++()
        {
            if ( offset < range->size() )
                offset += 1;
            return *this;
        }

        constexpr iterator operator++(int)
        {
            auto iter = *this;
            ++*this;
            return iter;
        }

        constexpr iterator& operator--()
        {
            if ( offset > 0 )
                offset -= 1;
            return *this;
        }

        constexpr iterator operator--(int)
        {
            auto iter = *this;
            --*this;
            return iter;
        }

        constexpr iterator& operator+=(difference_type off)
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

        constexpr iterator operator+(difference_type off) const
        {
            auto iter = *this;
            return iter += off;
        }

        friend constexpr iterator operator+(difference_type off, iterator iter)
        {
            return iter += off;
        }

        constexpr iterator& operator-=(difference_type off)
        {
            return *this += -off;
        }

        constexpr iterator operator-(difference_type off) const
        {
            auto iter = *this;
            return iter -= off;
        }

        constexpr difference_type operator-(const iterator& oth) const
        {
            return offset - oth.offset;
        }

        constexpr value_type operator[](difference_type off) const
        {
            return *(*this + off);
        }

        constexpr bool operator==(const iterator& oth) const
        {
            return (!valid() && !oth.valid()) ||
                   (range == oth.range && offset == oth.offset);
        }

        constexpr bool operator!=(const iterator& oth) const
        {
            return !(*this == oth);
        }

        constexpr bool valid() const
        {
            return range && offset <= range->size();
        }

        constexpr bool operator<(const iterator& oth) const
        {
            return offset < oth.offset;
        }

        constexpr bool operator<=(const iterator& oth) const
        {
            return offset <= oth.offset;
        }

        constexpr bool operator>(const iterator& oth) const
        {
            return offset > oth.offset;
        }

        constexpr bool operator>=(const iterator& oth) const
        {
            return offset >= oth.offset;
        }

    private:
        iterator(const BasicColorRange* range, size_type offset)
            : range(range), offset(offset)
        {}

        constexpr value_type color() const
        {
            using namespace repr;
            if ( range->size() < 2 )
                return range->first;
            return blend(range->first, range->second, double(offset) / (range->count - 1));
        }

        const BasicColorRange* range;
        size_type offset;
        friend BasicColorRange;
    };

    constexpr BasicColorRange(Repr first, Repr second, size_type count)
    : first(first), second(second), count(count)
    {}

    constexpr size_type size() const
    {
        return count;
    }

    constexpr void resize(size_type size)
    {
        count = size;
    }

    constexpr iterator begin() const
    {
        return iterator(this, 0);
    }

    constexpr iterator end() const
    {
        return iterator(this, count);
    }

    constexpr typename iterator::value_type operator[](size_type off) const
    {
        return begin()[off];
    }

private:
    Repr first;
    Repr second;
    size_type count;
};

using ColorRange = BasicColorRange<>;

} // namespace color
} // namespace melanolib
#endif // MELANOLIB_COLOR_RANGE_HPP
