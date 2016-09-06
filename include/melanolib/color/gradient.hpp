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
#ifndef MELANOLIB_COLOR_GRADIENT_HPP
#define MELANOLIB_COLOR_GRADIENT_HPP

#include <vector>

#include "melanolib/color/color.hpp"
#include "melanolib/color/color_iterator.hpp"

namespace melanolib {
namespace color {


template<class Repr=Color, class Container=std::vector<Repr>>
    class BasicGradient
{
public:
    using value_type = Repr;
    using container = Container;

    enum class OverflowMode
    {
        Clamp,
        Wrap,
        Mirror,
    };

    class GradientRange
    {
    public:
        using iterator = ColorIterator<BasicGradient, ConstantSize>;

        GradientRange(const BasicGradient& gradient, std::size_t size)
            : gradient(&gradient), size(size)
        {}

        iterator begin() const
        {
            return color::begin(*gradient, ConstantSize(size));
        }

        iterator end() const
        {
            return color::end(*gradient, ConstantSize(size));
        }

    private:
        const BasicGradient* gradient;
        std::size_t size;
    };

    template<class... Args>
        BasicGradient(Args&&... args)
            : _colors(std::forward<Args>(args)...)
        {}

    BasicGradient(std::initializer_list<value_type> args)
        : _colors(args)
    {}

    const container& colors() const
    {
        return _colors;
    }

    container& colors()
    {
        return _colors;
    }

    bool empty() const { return _colors.empty(); }
    auto size() const { return _colors.size(); }

    auto begin() { return _colors.begin(); }
    auto begin() const { return _colors.begin(); }
    auto cbegin() const { return _colors.cbegin(); }

    auto end() { return _colors.end(); }
    auto end() const { return _colors.end(); }
    auto cend() const { return _colors.cend(); }

    value_type color(float pos, OverflowMode flow = OverflowMode::Clamp) const
    {
        if ( pos < 0 )
        {
            if ( flow == OverflowMode::Wrap )
                pos = 1 + math::fractional(pos);
            else if ( flow == OverflowMode::Mirror )
                pos = mirror(pos <= -1 ? 1 - pos : -pos);
            else
                return _colors.front();
        }
        else if ( pos > 1 )
        {
            if ( flow == OverflowMode::Wrap )
                pos = math::fractional(pos);
            else if ( flow == OverflowMode::Mirror )
                pos = mirror(pos);
            else
                return _colors.back();
        }

        pos = math::denormalize<float>(pos, 0, size() - 1);
        std::size_t low = math::truncate<std::size_t>(pos);
        std::size_t high = math::ceil<std::size_t>(pos);

        if ( low == high )
            return _colors[low];

        using namespace repr;
        return blend(_colors[low], _colors[high], math::fractional(pos));
    }

    GradientRange range(std::size_t size) const
    {
        return GradientRange(*this, size);
    }

private:
    container _colors;

    static constexpr float mirror(float pos)
    {
        float mod = math::fractional(pos);
        return math::truncate(pos) % 2 ? 1 - mod : mod;
    }
};

using Gradient = BasicGradient<>;

} // namespace color
} // namespace melanolib
#endif // MELANOLIB_COLOR_GRADIENT_HPP
