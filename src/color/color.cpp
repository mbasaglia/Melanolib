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
 *
 */
#include "melanolib/color/color.hpp"

#include <tuple>

#include "melanolib/utils/c++-compat.hpp"
#include "melanolib/string/format.hpp"

namespace melanolib {
namespace color {

class ColorFormatFunctor
{

public:
    ColorFormatFunctor(const Color& color)
        : color(color)
    {}

    bool operator()(const std::string& key, const string::format::FormatSpec& spec, std::ostream& out)
    {
        using string::format::format_item;

        std::string color_space;
        std::string component;
        std::size_t dot = key.find('.');

        if ( dot == std::string::npos )
        {
            component = key;
        }
        else
        {
            color_space = key.substr(0, dot);
            component = key.substr(dot+1);
        }

        if ( component.empty() )
            return false;

        if ( component == "alpha" || (component == "a" && color_space != "lab") )
        {
            if ( spec.type_float() )
                return format_item(spec, color.alpha_float(), out);
            return format_item(spec, color.alpha(), out);
        }

        if ( color_space.empty() || color_space == "rgb" )
        {
            return rgb(component, spec, out);
        }
        else if ( color_space == "hsv" )
        {
            auto& hsv = get<repr::HSVf>();
            if ( component == "hue" || component == "h" )
                return format_item(spec, hsv.h, out);
            else if ( component == "saturation" || component == "sat" || component == "s" )
                return format_item(spec, hsv.s, out);
            else if ( component == "value" || component == "val" ||
                      component == "v" || component == "brightness" )
                return format_item(spec, hsv.v, out);
        }
        else if ( color_space == "lab" )
        {
            auto& lab = get<repr::Lab>();
            if ( component == "l" || component == "L" || component == "L*" )
                return format_item(spec, lab.l, out);
            else if ( component == "a" || component == "a*" )
                return format_item(spec, lab.a, out);
            else if ( component == "b" || component == "b*" )
                return format_item(spec, lab.b, out);
        }
        else if ( color_space == "xyz" )
        {
            auto& xyz = get<repr::XYZ>();
            if ( component == "x" || component == "X" )
                return format_item(spec, xyz.x, out);
            else if ( component == "y" || component == "Y" )
                return format_item(spec, xyz.y, out);
            else if ( component == "z" || component == "Z" )
                return format_item(spec, xyz.z, out);
        }

        return false;
    }

private:

    bool rgb(const std::string& component, const string::format::FormatSpec& spec, std::ostream& out)
    {
        using string::format::format_item;

        if ( component == "r" || component == "red" )
        {
            if ( spec.type_float() )
                return format_item(spec, get<repr::RGBf>().r, out);
            return format_item(spec, color.red(), out);
        }
        else if ( component == "g" || component == "green" )
        {
            if ( spec.type_float() )
                return format_item(spec, get<repr::RGBf>().g, out);
            return format_item(spec, color.green(), out);
        }
        else if ( component == "b" || component == "blue" )
        {
            if ( spec.type_float() )
                return format_item(spec, get<repr::RGBf>().b, out);
            return format_item(spec, color.blue(), out);
        }
        else if ( component == "int24" || component == "int24.rgb" )
        {
            return format_item(spec, get<repr::RGB_int24>().rgb, out);
        }
        else if ( component == "int24.rgba" )
        {
            return format_item(spec, get<repr::RGB_int24>().rgba(color.alpha()), out);
        }
        else if ( component == "int24.argb" )
        {
            return format_item(spec, get<repr::RGB_int24>().argb(color.alpha()), out);
        }
        else if ( component == "int12" || component == "int12.rgb" )
        {
            return format_item(spec, get<repr::RGB_int12>().rgb, out);
        }
        else if ( component == "int12.rgba" )
        {
            auto alpha = (color.alpha() & 0xf0) >> 4;
            return format_item(spec, get<repr::RGB_int12>().rgba(alpha), out);
        }
        else if ( component == "int12.argb" )
        {
            auto alpha = (color.alpha() & 0xf0) >> 4;
            return format_item(spec, get<repr::RGB_int12>().argb(alpha), out);
        }
        else if ( component == "int3" || component == "int3.rgb" )
        {
            return format_item(spec, get<repr::RGB_int3>().rgb(), out);
        }
        else if ( component == "int3.bright" )
        {
            return format_item(spec, get<repr::RGB_int3>().bright(), out);
        }

        return false;
    }

    template<class Repr>
    Repr& get()
    {
        auto& optional = std::get<Optional<Repr>>(representations);
        if ( !optional )
            optional = color.to<Repr>();
        return *optional;
    }

    Color color;
    std::tuple<
        Optional<repr::RGBf>,
        Optional<repr::HSVf>,
        Optional<repr::RGB_int12>,
        Optional<repr::RGB_int24>,
        Optional<repr::RGB_int3>,
        Optional<repr::Lab>,
        Optional<repr::XYZ>
    > representations;
};


std::string Color::format(const std::string& template_string) const
{
    return string::format::sformat(template_string, ColorFormatFunctor(*this));
}

} // namespace color
} // namespace melanolib
