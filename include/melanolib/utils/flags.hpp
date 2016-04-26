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
#ifndef MELANOLIB_UTILS_FLAGS_HPP
#define MELANOLIB_UTILS_FLAGS_HPP

#include <type_traits>
#include <iostream>

#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {

/**
 * \brief Enum wrapper for flag masks
 * \todo Tests
 */
template<class Enum, int Default=0>
    class Flags
{
public:
    using enum_type = Enum;
    using int_type = std::underlying_type_t<Enum>;

    constexpr Flags(enum_type flags) noexcept : flags(int_type(flags)) {}

    constexpr Flags() noexcept = default;

    constexpr Flags(int_type flags) noexcept : flags(flags) {}

    template<int OtherDefault>
        constexpr Flags(Flags<Enum, OtherDefault> flags) noexcept
        : flags(int_type(flags)) {}

    explicit constexpr operator bool() const noexcept { return flags; }

    explicit constexpr operator int_type() const noexcept { return flags; }

    constexpr Flags operator|(const Flags& o) const noexcept
    {
        return flags | o.flags;
    }

    constexpr Flags operator&(const Flags& o) const noexcept
    {
        return flags & o.flags;
    }
    constexpr Flags operator&(int_type o) const noexcept
    {
        return flags & o;
    }

    constexpr Flags operator^(const Flags& o) const noexcept
    {
        return flags ^ o.flags;
    }

    constexpr Flags operator~() const noexcept
    {
        return ~flags;
    }

    constexpr bool operator==(const Flags& o) const noexcept
    {
        return flags == o.flags;
    }

    SUPER_CONSTEXPR Flags& operator|=(const Flags& o) noexcept
    {
        flags |= o.flags;
        return *this;
    }

    SUPER_CONSTEXPR Flags& operator&=(const Flags& o) noexcept
    {
        flags &= o.flags;
        return *this;
    }

    SUPER_CONSTEXPR Flags& operator^=(const Flags& o) noexcept
    {
        flags ^= o.flags;
        return *this;
    }

    constexpr bool has_flag(Flags v) noexcept
    {
        return (flags & v) == v;
    }

    SUPER_CONSTEXPR void enable_flags(Flags v) noexcept
    {
        flags |= v.flags;
    }

    SUPER_CONSTEXPR void disable_flags(Flags v) noexcept
    {
        flags &= ~v.flags;
    }

    friend std::ostream& operator<<(std::ostream& os, const Flags& f)
    {
        return os << f.flags;
    }

private:
    int_type flags = Default;
};

namespace enum_operators {

template<class Enum>
    constexpr Flags<Enum> operator| (Enum a, Enum b) noexcept
    {
        return Flags<Enum>(a) | Flags<Enum>(b);
    }

} // namespace enum_operators

} // namespace melanolib
#endif // MELANOLIB_UTILS_FLAGS_HPP
