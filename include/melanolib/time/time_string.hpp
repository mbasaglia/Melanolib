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
#ifndef TIME_STRING_HPP
#define TIME_STRING_HPP

#include "melanolib/time/date_time.hpp"
#include "melanolib/string/language.hpp"
#include "melanolib/string/simple_stringutils.hpp"

namespace melanolib {
namespace time {

/**
 * \brief Full month name in English
 */
std::string month_name(Month month);

/**
 * \brief Short month name in English
 */
std::string month_shortname(Month month);

/**
 * \brief Month from English name
 */
melanolib::Optional<Month> month_from_name(const std::string& name);

/**
 * \brief Full weekday name in English
 */
std::string weekday_name(WeekDay day);

/**
 * \brief Short weekday name in English
 */
std::string weekday_shortname(WeekDay day);

/**
 * \brief Weekday from English name
 */
melanolib::Optional<WeekDay> weekday_from_name(const std::string& name);

/**
 * \brief Returns a string from a time format character
 * \see http://php.net/manual/en/function.date.php
 */
std::string format_char(const DateTime& date_time, char c);

/**
 * \brief Format a string from DateTime with the given format
 *
 * The character \\ escapes characters so they won't be expanded
 * \see http://php.net/manual/en/function.date.php
 */
std::string format(const DateTime& date_time, const std::string& fmt);

/**
 * \brief Equivalent to format(DateTime(), fmt)
 */
inline std::string format(const std::string& fmt)
{
    return format(DateTime(), fmt);
}

/**
 * \brief Format a string from DateTime with the given format
 *
 * Similar to format() but special characters must be preceded by %
 */
std::string strftime(const DateTime& date_time, const std::string& fmt);

/**
 * \brief Equivalent to format(DateTime(), fmt)
 */
inline std::string strftime(const std::string& fmt)
{
    return strftime(DateTime(), fmt);
}

/**
 * \brief Converts a duration to a string
 * \todo Unit test
 */
template<class Rep, class Period>
    std::string duration_string(const std::chrono::duration<Rep, Period>& duration)
{
    auto dursec = std::chrono::duration_cast<seconds>(duration).count();
    std::vector<std::string> durtext;
    durtext.reserve(5);
    using string::english;

    if ( dursec % 60 )
        durtext.push_back(english.pluralize_with_number(dursec%60, "second"));

    dursec /= 60;
    if ( dursec )
        durtext.push_back(english.pluralize_with_number(dursec%60, "minute"));

    dursec /= 60;
    if ( dursec )
        durtext.push_back(english.pluralize_with_number(dursec%24, "hour"));

    dursec /= 24;
    if ( dursec % 7 )
        durtext.push_back(english.pluralize_with_number(dursec%7, "day"));

    dursec /= 7;
    if ( dursec )
        durtext.push_back(english.pluralize_with_number(dursec, "week"));

    std::reverse(durtext.begin(), durtext.end());
    return string::implode(" ", durtext);
}

template<class Rep, class Period>
    std::string duration_string_short(const std::chrono::duration<Rep, Period>& duration)
{
    auto dursec = std::chrono::duration_cast<seconds>(duration).count();
    std::string durtext;
    using string::english;

    durtext = string::to_string(dursec%60, 2) + durtext;

    dursec /= 60;
    durtext = string::to_string(dursec%60, 2) + ':' + durtext;

    dursec /= 60;
    if ( dursec )
        durtext = string::to_string(dursec%60, 2) + ':' + durtext;

    dursec /= 24;
    if ( dursec % 7 )
        durtext = english.pluralize_with_number(dursec%7, "day")+' '+durtext;

    return durtext;
}

} // namespace timer
} // namespace melanolib

#endif // TIME_STRING_HPP
