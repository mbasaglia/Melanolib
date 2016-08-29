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
#ifndef MELANOLIB_COLOR_COLOR_HPP
#define MELANOLIB_COLOR_COLOR_HPP

#include <cstdint>
#include <string>
#include <ostream>
#include "melanolib/math/math.hpp"
#include "melanolib/math/vector.hpp"

namespace melanolib {
namespace color {

/**
 * \brief Color representation namespace
 */
namespace repr {

/**
 * \brief 24bit interger RGB
 */
struct RGB
{
    uint8_t r, g, b;

    constexpr RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    constexpr RGB() : r(0), g(0), b(0) {}
};

/**
 * \brief Floating-point RGB
 */
struct RGBf
{
    float r, g, b;

    constexpr RGBf(float r, float g, float b) : r(r), g(g), b(b) {}
    constexpr RGBf(const melanolib::math::Vec3f& v) : r(v[0]), g(v[1]), b(v[2]) {}
    constexpr melanolib::math::Vec3f vec() const { return {r, g, b}; }
};

/**
 * \brief Floating-point HSV
 */
struct HSVf
{
    float h, s, v;

    constexpr HSVf(float h, float s, float v) : h(h), s(s), v(v) {}
    constexpr HSVf(const melanolib::math::Vec3f& v) : h(v[0]), s(v[1]), v(v[2]) {}
    constexpr melanolib::math::Vec3f vec() const { return {h, s, v}; }
};

/**
 * \brief CIE L*a*b
 * L* [0, 100]
 * a* [-128, 127]
 * b* [-128, 127]
 */
struct Lab
{
    float l, a, b;

    constexpr Lab(float l, float a, float b) : l(l), a(a), b(b) {}
    constexpr Lab(const melanolib::math::Vec3f& v) : l(v[0]), a(v[1]), b(v[2]) {}
    constexpr melanolib::math::Vec3f vec() const { return {l, b, b}; }
};

/**
 * \brief CIE XYZ
 */
struct XYZ
{
    float x, y, z;

    constexpr XYZ(float x, float y, float z) : x(x), y(y), z(z) {}
    constexpr XYZ(const melanolib::math::Vec3f& v) : x(v[0]), y(v[1]), z(v[2]) {}
    constexpr melanolib::math::Vec3f vec() const { return {x, y, z}; }
};

/**
 * \brief Similar to RGB but represented as a single integer
 */
struct RGB_int24
{
    uint32_t rgb;
    constexpr RGB_int24(uint32_t rgb) : rgb(rgb) {}

    constexpr uint32_t rgba(uint8_t alpha = 255) const
    {
        return (rgb << 8) | alpha;
    }

    constexpr uint32_t argb(uint8_t alpha = 255) const
    {
        return (alpha << 24) | rgb;
    }
};

/**
 * \brief Similar to RGB but with a 12 bit integer
 */
struct RGB_int12
{
    uint16_t rgb;
    constexpr RGB_int12(uint16_t rgb) : rgb(rgb) {}

    constexpr uint16_t rgba(uint8_t alpha = 0xf) const
    {
        return (rgb << 4) | alpha;
    }

    constexpr uint16_t argb(uint8_t alpha = 0xf) const
    {
        return (alpha << 12) | rgb;
    }
};

/**
 * \brief RGB with a single bit per channel plus one bit for brightness
 */
struct RGB_int3
{
    uint8_t color;

    constexpr RGB_int3(uint8_t rgb, bool bright = false)
        : color(rgb)
    {
        if ( bright )
            color |= 0b1000;
    }

    constexpr bool red() const
    {
        return color & 0b0001;
    }

    constexpr bool green() const
    {
        return color & 0b0010;
    }

    constexpr bool blue() const
    {
        return color & 0b0100;
    }

    constexpr bool bright() const
    {
        return color & 0b1000;
    }

    constexpr uint8_t rgb() const
    {
        return color & 0b0111;
    }
};

} // namespace repr

class Color
{
public:

    template<class Repr>
        constexpr Color(Repr repr, uint8_t alpha = 255)
        : _alpha(alpha)
    {
        from(repr);
    }

    template<class Repr, class Float, class = std::enable_if_t<std::is_floating_point<Float>::value>>
        constexpr Color(Repr repr, Float alpha)
        : _alpha(melanolib::math::round<uint8_t>(alpha * 255))
    {
        from(repr);
    }

    constexpr Color()
        : _alpha(0), _valid(false)
    {}

    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : _rgb(r, g, b), _alpha(a)
    {}

    constexpr auto alpha() const
    {
        return _alpha;
    }

    constexpr auto red() const
    {
        return _rgb.r;
    }

    constexpr auto green() const
    {
        return _rgb.g;
    }

    constexpr auto blue() const
    {
        return _rgb.b;
    }

    constexpr auto valid() const
    {
        return _valid;
    }

    constexpr float alpha_float() const
    {
        return _alpha / 255.f;
    }

    /**
     * \brief Convert the color to a different color sace
     * \note This operation is only defined for valid colors
     */
    template<class Repr>
        Repr to() const;

    constexpr bool operator==(const Color& oth) const
    {
        return (!_valid && !oth._valid) || (
            _rgb.r == oth._rgb.r &&
            _rgb.g == oth._rgb.g &&
            _rgb.b == oth._rgb.b &&
            _alpha == oth._alpha
        );
    }

    constexpr bool operator!=(const Color& oth) const
    {
        return !(*this == oth);
    }

    friend std::ostream& operator<< (std::ostream& os, const Color& color)
    {
        if ( !color._valid )
            return os << "rgb()";

        if ( color._alpha == 255 )
            return os << "rgb("
                      << int(color._rgb.r) << ", "
                      << int(color._rgb.g) << ", "
                      << int(color._rgb.b) << ")";

        return os << "rgba("
                  << int(color._rgb.r) << ", "
                  << int(color._rgb.g) << ", "
                  << int(color._rgb.b) << ", "
                  << int(color._alpha) << ")";
    }

    /**
     * \brief Distance between two colors
     * Uses Lab color space to determine the distance
     * \note This operation is only defined for valid colors
     */
    float distance(const Color& oth) const;

    template<class Repr=repr::RGBf>
        constexpr Color blend(const Color& oth, float factor = 0.5) const
    {
        using namespace melanolib::math;
        return Color(
            Repr(linear_interpolation(to<Repr>().vec(), oth.to<Repr>().vec(), factor)),
            linear_interpolation(alpha_float(), oth.alpha_float(), factor)
        );
    }

    /**
     * \brief Formats the color according to a template string
     * \see melanolib::format::format()
     */
    std::string format(const std::string& template_string = "#{r:02x}{g:02x}{b:02x}") const;

private:
    template<class Repr>
        void from(Repr value);

private:
    repr::RGB _rgb;
    uint8_t   _alpha;
    bool      _valid = true;
};

template<>
    inline constexpr void Color::from<repr::RGB>(repr::RGB value)
{
    _rgb = value;
}

template<>
    inline constexpr void Color::from<repr::RGBf>(repr::RGBf value)
{
    _rgb = repr::RGB{
        melanolib::math::round<uint8_t>(value.r * 255),
        melanolib::math::round<uint8_t>(value.g * 255),
        melanolib::math::round<uint8_t>(value.b * 255)
    };
}

template<>
    inline constexpr void Color::from<repr::HSVf>(repr::HSVf value)
{
    using namespace melanolib::math;

    auto h = value.h;
    if ( h < 0 )
        h = 0;
    else if ( h > 1 )
        h = fractional(h);
    h *= 6;

    auto s = bound(0.f, value.s, 1.f);
    auto v = bound(0.f, value.v, 1.f);

    auto c = v * s;
    auto m = v - c;

    auto h1 = truncate(h);
    auto f = h - h1;
    auto n = v - c * f;
    auto k = v - c * (1 - f);

    auto iv = round<uint8_t>(v * 255);
    auto im = round<uint8_t>(m * 255);
    auto in = round<uint8_t>(n * 255);
    auto ik = round<uint8_t>(k * 255);

    switch ( h1 )
    {
        case 0:
            _rgb = repr::RGB(iv, ik, im);
            break;
        case 1:
            _rgb = repr::RGB(in, iv, im);
            break;
        case 2:
            _rgb = repr::RGB(im, iv, ik);
            break;
        case 3:
            _rgb = repr::RGB(im, in, iv);
            break;
        case 4:
            _rgb = repr::RGB(ik, im, iv);
            break;
        case 5:
            _rgb = repr::RGB(iv, im, in);
            break;
        case 6:
        default:
            _rgb = repr::RGB(iv, ik, im);
            break;
    }
}

template<>
    inline constexpr void Color::from<repr::RGB_int24>(repr::RGB_int24 value)
{
    _rgb.r = (value.rgb >> 16) & 0xff;
    _rgb.g = (value.rgb >> 8) & 0xff;
    _rgb.b = value.rgb & 0xff;
}

template<>
    inline constexpr void Color::from<repr::RGB_int12>(repr::RGB_int12 value)
{
    _rgb.r = (value.rgb >> 8) & 0xf;
    _rgb.r = _rgb.r | (_rgb.r << 4);
    _rgb.g = (value.rgb >> 4) & 0xf;
    _rgb.g = _rgb.g | (_rgb.g << 4);
    _rgb.b = value.rgb & 0xf;
    _rgb.b = _rgb.b | (_rgb.b << 4);
}

template<>
    inline constexpr void Color::from<repr::RGB_int3>(repr::RGB_int3 value)
{
    if ( value.rgb() == 0b000 )
    {
        _rgb.r = _rgb.g = _rgb.b = value.bright() ? 70 : 0;
    }
    else if ( value.rgb() == 0b111 )
    {
        _rgb.r = _rgb.g = _rgb.b = value.bright() ? 255 : 136;
    }
    else
    {
        uint8_t val = value.bright() ? 255 : 128;
        _rgb.r = value.red() ? val : 0;
        _rgb.g = value.green() ? val : 0;
        _rgb.b = value.blue() ? val : 0;
    }
}

template<>
    inline void Color::from<repr::XYZ>(repr::XYZ value)
{
    value.x /= 100;
    value.y /= 100;
    value.z /= 100;

    float r = value.x *  3.2406 + value.y * -1.5372 + value.z * -0.4986;
    float g = value.x * -0.9689 + value.y *  1.8758 + value.z *  0.0415;
    float b = value.x *  0.0557 + value.y * -0.2040 + value.z *  1.0570;

    auto conv = [](float v)
    {
        return v > 0.0031308 ?
            1.055 * melanolib::math::pow(v, 1 / 2.4) - 0.055 :
            12.92 * v;
    };

    _rgb.r = melanolib::math::round<uint8_t>(conv(r) * 255);
    _rgb.g = melanolib::math::round<uint8_t>(conv(g) * 255);
    _rgb.b = melanolib::math::round<uint8_t>(conv(b) * 255);
}

template<>
    inline void Color::from<repr::Lab>(repr::Lab value)
{
    repr::XYZ ref{95.047, 100.000, 108.883};

    float y = ( value.l + 16 ) / 116;
    float x = value.a / 500 + y;
    float z = y - value.b / 200;

    auto conv = [](float v)
    {
        auto v3 = melanolib::math::pow(v, 3);
        return v3 > 0.008856 ? v3 : (v - 16.0 / 116) / 7.787;
    };

    from(repr::XYZ(conv(x) * ref.x, conv(y) * ref.y, conv(z) * ref.z));
}


template<>
    inline constexpr repr::RGBf Color::to<repr::RGBf>() const
{
    return {
        _rgb.r / 255.0f,
        _rgb.g / 255.0f,
        _rgb.b / 255.0f,
    };
}

template<>
    inline constexpr repr::HSVf Color::to<repr::HSVf>() const
{
    auto rgbf = to<repr::RGBf>();

    auto cmax = melanolib::math::max(rgbf.r, rgbf.g, rgbf.b);
    auto cmin = melanolib::math::min(rgbf.r, rgbf.g, rgbf.b);
    float h = 0;
    auto delta = cmax - cmin;

    if ( delta > 0 )
    {

        if ( cmax == rgbf.r )
            h = rgbf.g - rgbf.b / delta;
        else if ( cmax == rgbf.g )
            h = (rgbf.b - rgbf.r) / delta + 2;
        else // cmax == b
            h = (rgbf.r - rgbf.g) / delta + 4;

        if ( h < 0 )
            h = 6 + h;
    }

    h /= 6;
    auto s = cmax > 0 ? delta / cmax : 0;
    auto v = cmax;

    return {h, s, v};
}

template<>
    inline repr::XYZ Color::to<repr::XYZ>() const
{
    auto rgbf = to<repr::RGBf>();

    auto conv = [](float v)
    {
        return v > 0.04045 ? melanolib::math::pow((v + 0.055) / 1.055, 2.4) : v / 12.92;
    };

    rgbf.r = conv(rgbf.r) * 100;
    rgbf.g = conv(rgbf.g) * 100;
    rgbf.b = conv(rgbf.b) * 100;

    return {
        rgbf.r * 0.4124f + rgbf.g * 0.3576f + rgbf.b * 0.1805f,
        rgbf.r * 0.2126f + rgbf.g * 0.7152f + rgbf.b * 0.0722f,
        rgbf.r * 0.0193f + rgbf.g * 0.1192f + rgbf.b * 0.9505f
    };
}

template<>
    inline repr::Lab Color::to<repr::Lab>() const
{
    auto source = to<repr::XYZ>();

    repr::XYZ ref{95.047, 100.000, 108.883};

    auto conv = [](float v) -> float
    {
        return v > 0.008856 ? melanolib::math::pow(v, 1.0 / 3) : 7.787 * v + 16.0 / 116;
    };

    repr::XYZ relative{
        conv(source.x / ref.x),
        conv(source.y / ref.y),
        conv(source.z / ref.z),
    };
    return {
        (116 * relative.y) - 16,
        500 * (relative.x - relative.y),
        200 * (relative.y - relative.z)
    };
}

template<>
    inline constexpr repr::RGB_int24 Color::to<repr::RGB_int24>() const
{
    return repr::RGB_int24((_rgb.r << 16) | (_rgb.g << 8) | _rgb.b);
}

template<>
    inline constexpr repr::RGB_int12 Color::to<repr::RGB_int12>() const
{
    return repr::RGB_int12(
        ((_rgb.r & 0xf0) << 4) |
        (_rgb.g & 0xf0 ) |
        ((_rgb.b & 0xf0) >> 4)
    );
}

template<>
    inline constexpr repr::RGB_int3 Color::to<repr::RGB_int3>() const
{
    auto hsv = to<repr::HSVf>();

    if ( hsv.s >= 0.3 )
    {
        float hue = hsv.h * 6;

        uint8_t color = 0;

        if ( hue <= 0.5 )      color = 0b001; // red
        else if ( hue <= 1.5 ) color = 0b011; // yellow
        else if ( hue <= 2.5 ) color = 0b010; // green
        else if ( hue <= 3.5 ) color = 0b110; // cyan
        else if ( hue <= 4.5 ) color = 0b100; // blue
        else if ( hue <= 5.5 ) color = 0b101; // magenta
        else                   color = 0b001; // red

        return repr::RGB_int3(color, hsv.v > 0.6);
    }

    if ( hsv.v > 0.8 )
        return repr::RGB_int3(0b1111); // white
    else if ( hsv.v > 0.5 )
        return repr::RGB_int3(0b0111); // silver
    else if ( hsv.v > 0.25 )
        return repr::RGB_int3(0b1000); // gray
    else
        return repr::RGB_int3(0b0000); // black

}


/**
 * \brief CIE76 Delta-E distance between two Lab colors
 */
inline float delta_e(const repr::Lab& a, const repr::Lab& b)
{
    return melanolib::math::sqrt(
        melanolib::math::pow(a.l - b.l, 2) +
        melanolib::math::pow(a.a - b.a, 2) +
        melanolib::math::pow(a.b - b.b, 2)
    );
}

inline float Color::distance(const Color& oth) const
{
    return delta_e(to<repr::Lab>(), oth.to<repr::Lab>());
}

} // namespace color
} // namespace melanolib
#endif // MELANOLIB_COLOR_COLOR_HPP
