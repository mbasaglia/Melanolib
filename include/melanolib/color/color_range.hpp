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
#include "melanolib/color/color_iterator.hpp"

namespace melanolib {
namespace color {

template<class Repr=Color>
    class BasicColorRange
{
public:
    using iterator = ColorIterator<BasicColorRange, Repr>;
    using value_type = typename iterator::value_type;
    using size_type = typename iterator::size_type;

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
        return iterator::begin(*this);
    }

    constexpr iterator end() const
    {
        return iterator::end(*this);
    }

    constexpr value_type operator[](size_type off) const
    {
        return begin()[off];
    }

    constexpr value_type color(float factor) const
    {
        using namespace repr;
        if ( size() < 2 || factor < 0 )
            return first;
        if ( factor > 1 )
            return second;
        return blend(first, second, factor);
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
