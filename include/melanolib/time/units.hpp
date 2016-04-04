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
#ifndef MELANOLIB_TIME_UNITS_HPP
#define MELANOLIB_TIME_UNITS_HPP

#include <chrono>
#include <cstdint>

#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {
/**
 * \brief Namespace for time utilities
 */
namespace time {

using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;
using days = std::chrono::duration<int64_t, std::ratio<(24*3600)>>;
using weeks = std::chrono::duration<int64_t, std::ratio<(24*3600*7)>>;

/**
 * \brief Month enum
 */
enum class Month : uint8_t {
    JANUARY  = 1,
    FEBRUARY = 2,
    MARCH    = 3,
    APRIL    = 4,
    MAY      = 5,
    JUNE     = 6,
    JULY     = 7,
    AUGUST   = 8,
    SEPTEMBER= 9,
    OCTOBER  =10,
    NOVEMBER =11,
    DECEMBER =12
};

constexpr Month operator- (Month m, int i) noexcept;
inline constexpr Month operator+ (Month m, int i) noexcept
{
    return i > 0 ? Month((((int(m)-1)+i)%12)+1) : m - -i;
}

inline SUPER_CONSTEXPR Month& operator+= (Month& m, int i) noexcept
{
    m = m + i;
    return m;
}
inline constexpr Month operator- (Month m, int i) noexcept
{
    return i < 0 ? m + -i : ( i < int(m) ? Month(int(m)-i) : m - (i-12) );
}

inline SUPER_CONSTEXPR Month& operator-= (Month& m, int i) noexcept
{
    m = m - i;
    return m;
}

inline SUPER_CONSTEXPR Month& operator++ (Month& m) noexcept
{
    return m += 1;
}

inline SUPER_CONSTEXPR Month operator++ (Month& m, int) noexcept
{
    auto c = m;
    ++m;
    return c;
}

inline SUPER_CONSTEXPR Month& operator-- (Month& m) noexcept
{
    return m -= 1;
}

inline SUPER_CONSTEXPR Month operator-- (Month& m, int) noexcept
{
    auto c = m;
    --m;
    return c;
}

/**
 * \brief Week day enum
 */
enum class WeekDay : uint8_t {
    MONDAY      = 1,
    TUESDAY     = 2,
    WEDNESDAY   = 3,
    THURSDAY    = 4,
    FRIDAY      = 5,
    SATURDAY    = 6,
    SUNDAY      = 7
};

constexpr WeekDay operator- (WeekDay m, int i) noexcept;
inline constexpr WeekDay operator+ (WeekDay m, int i) noexcept
{
    return i > 0 ? WeekDay((((int(m)-1)+i)%7)+1) : m - -i;
}

inline SUPER_CONSTEXPR WeekDay& operator+= (WeekDay& m, int i) noexcept
{
    m = m + i;
    return m;
}
inline constexpr WeekDay operator- (WeekDay m, int i) noexcept
{
    return i < 0 ? m + -i : ( i < int(m) ? WeekDay(int(m)-i) : m - (i-7) );
}

inline SUPER_CONSTEXPR WeekDay& operator-= (WeekDay& m, int i) noexcept
{
    m = m - i;
    return m;
}

inline SUPER_CONSTEXPR WeekDay& operator++ (WeekDay& m) noexcept
{
    return m += 1;
}

inline SUPER_CONSTEXPR WeekDay operator++ (WeekDay& m, int) noexcept
{
    auto c = m;
    ++m;
    return c;
}

inline SUPER_CONSTEXPR WeekDay& operator-- (WeekDay& m) noexcept
{
    return m -= 1;
}

inline SUPER_CONSTEXPR WeekDay operator-- (WeekDay& m, int) noexcept
{
    auto c = m;
    --m;
    return c;
}

/**
 * \brief Converts between time points belongong to different clocks
 */
template<class TimeTo, class TimeFrom>
    TimeTo time_point_convert(TimeFrom&& tp)
{
    return TimeTo::clock::now() + (tp - TimeFrom::clock::now());
}

} // namespace time
} // namespace melanolib

#endif // MELANOLIB_TIME_UNITS_HPP
