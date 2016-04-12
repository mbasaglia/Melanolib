/**
 * \file
 * \author Mattia Basaglia
 * \copyright Copyright 2015-2016 Mattia Basaglia
 * \section License
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define BOOST_TEST_MODULE Test_Time

#include <boost/test/unit_test.hpp>

#include "melanolib/time/date_time.hpp"
#include "melanolib/time/time_string.hpp"
#include "melanolib/time/time_parser.hpp"

using namespace melanolib::time;

BOOST_AUTO_TEST_CASE( test_Month )
{
    Month m = Month::JANUARY;
    BOOST_CHECK( int(m) == 1 );
    ++m;
    BOOST_CHECK( int(m) == 2 );
    m++;
    BOOST_CHECK( int(m) == 3 );
    m += 4;
    BOOST_CHECK( int(m) == 7 );
    m += 13;
    BOOST_CHECK( int(m) == 8 );
    --m;
    BOOST_CHECK( int(m) == 7 );
    m--;
    BOOST_CHECK( int(m) == 6 );
    m -= 4;
    BOOST_CHECK( int(m) == 2 );
    m -= 13;
    BOOST_CHECK( int(m) == 1 );

    BOOST_CHECK( Month::MAY - -2 == Month::JULY );
    BOOST_CHECK( Month::MAY + -2 ==  Month::MARCH );

}

BOOST_AUTO_TEST_CASE( test_WeekDay )
{
    WeekDay m = WeekDay::MONDAY;
    BOOST_CHECK( int(m) == 1 );
    ++m;
    BOOST_CHECK( int(m) == 2 );
    m++;
    BOOST_CHECK( int(m) == 3 );
    m += 2;
    BOOST_CHECK( int(m) == 5 );
    m += 8;
    BOOST_CHECK( int(m) == 6 );
    --m;
    BOOST_CHECK( int(m) == 5 );
    m--;
    BOOST_CHECK( int(m) == 4 );
    m -= 2;
    BOOST_CHECK( int(m) == 2 );
    m -= 8;
    BOOST_CHECK( int(m) == 1 );

    BOOST_CHECK( WeekDay::WEDNESDAY - -2 == WeekDay::FRIDAY );
    BOOST_CHECK( WeekDay::WEDNESDAY + -2 ==  WeekDay::MONDAY );

}

BOOST_AUTO_TEST_CASE( test_DateTime )
{
    // explicit ctor
    DateTime mlps5utc(2015,Month::APRIL,days(4),hours(15),minutes(0));
    BOOST_CHECK( mlps5utc.year() == 2015 );
    BOOST_CHECK( mlps5utc.month() == Month::APRIL );
    BOOST_CHECK( mlps5utc.month_int() == 4 );
    BOOST_CHECK( mlps5utc.day() == 4 );
    BOOST_CHECK( mlps5utc.hour() == 15 );
    BOOST_CHECK( mlps5utc.minute() == 0 );
    BOOST_CHECK( mlps5utc.second() == 0 );
    BOOST_CHECK( mlps5utc.millisecond() == 0 );

    // explicit ctor bounding overflows
    DateTime overflowctor(2015,Month(15),days(34),hours(25),minutes(70),seconds(346),milliseconds(7777));
    BOOST_CHECK( overflowctor.year() == 2015 );
    BOOST_CHECK( overflowctor.month() <= Month::DECEMBER );
    BOOST_CHECK( overflowctor.day() <= 31 );
    BOOST_CHECK( overflowctor.hour() < 24 );
    BOOST_CHECK( overflowctor.minute() < 60 );
    BOOST_CHECK( overflowctor.second() < 60 );
    BOOST_CHECK( overflowctor.millisecond() < 1000 );

    // month days
    BOOST_CHECK( DateTime::month_days(2015,Month::JANUARY)      == 31 );
    BOOST_CHECK( DateTime::month_days(2015,Month::FEBRUARY)     == 28 );
    BOOST_CHECK( DateTime::month_days(2015,Month::MARCH)        == 31 );
    BOOST_CHECK( DateTime::month_days(2015,Month::APRIL)        == 30 );
    BOOST_CHECK( DateTime::month_days(2015,Month::MAY)          == 31 );
    BOOST_CHECK( DateTime::month_days(2015,Month::JUNE)         == 30 );
    BOOST_CHECK( DateTime::month_days(2015,Month::JULY)         == 31 );
    BOOST_CHECK( DateTime::month_days(2015,Month::AUGUST)       == 31 );
    BOOST_CHECK( DateTime::month_days(2015,Month::SEPTEMBER)    == 30 );
    BOOST_CHECK( DateTime::month_days(2015,Month::OCTOBER)      == 31 );
    BOOST_CHECK( DateTime::month_days(2015,Month::NOVEMBER)     == 30 );
    BOOST_CHECK( DateTime::month_days(2015,Month::DECEMBER)     == 31 );

    // leap years
    BOOST_CHECK( !DateTime::leap_year(2015) );
    BOOST_CHECK( DateTime::month_days(2015,Month::FEBRUARY) == 28 );
    BOOST_CHECK( DateTime::leap_year(2012) );
    BOOST_CHECK( DateTime::month_days(2012,Month::FEBRUARY) == 29 );
    BOOST_CHECK( DateTime::leap_year(2000) );
    BOOST_CHECK( DateTime::month_days(2000,Month::FEBRUARY) == 29 );
    BOOST_CHECK( DateTime::leap_year(2004) );
    BOOST_CHECK( DateTime::month_days(2004,Month::FEBRUARY) == 29 );
    BOOST_CHECK( !DateTime::leap_year(2100) );
    BOOST_CHECK( DateTime::month_days(2100,Month::FEBRUARY) == 28 );
    BOOST_CHECK( DateTime::leap_year(2400) );
    BOOST_CHECK( DateTime::month_days(2400,Month::FEBRUARY) == 29 );


    // operations
    DateTime time(2015,Month::DECEMBER,days(31),hours(23),minutes(59));
    // \todo simple setters

    // + millisecond (no overflow)
    time += milliseconds(500);
    BOOST_CHECK( time.millisecond() == 500 );
    // + millisecond (overflow)
    time += milliseconds(500);
    BOOST_CHECK( time.millisecond() == 0 );
    BOOST_CHECK( time.second() == 1 );

    // + seconds (no overflow)
    time += seconds(58);
    // + seconds (full overflow)
    BOOST_CHECK( time.second() == 59 );
    time += seconds(1);
    BOOST_CHECK( time.year() == 2016 );
    BOOST_CHECK( time.month() == Month::JANUARY );
    BOOST_CHECK( time.day() == 1 );
    BOOST_CHECK( time.hour() == 0 );
    BOOST_CHECK( time.minute() == 0 );
    BOOST_CHECK( time.second() == 0 );
    BOOST_CHECK( time.millisecond() == 0 );
    BOOST_CHECK( time == DateTime(2016,Month::JANUARY,days(1),hours(0),minutes(0),seconds(0),milliseconds(0)) );

    // + hours (no overflow)
    time += hours(4);
    BOOST_CHECK( time == DateTime(2016,Month::JANUARY,days(1),hours(4),minutes(0),seconds(0),milliseconds(0)) );
    // + hours (overflow)
    time += hours(25);
    BOOST_CHECK( time == DateTime(2016,Month::JANUARY,days(2),hours(5),minutes(0),seconds(0),milliseconds(0)) );

    // + days (no overflow)
    time += days(29);
    BOOST_CHECK( time == DateTime(2016,Month::JANUARY,days(31),hours(5),minutes(0),seconds(0),milliseconds(0)) );
    // + days (overflow)
    time += days(7);
    BOOST_CHECK( time == DateTime(2016,Month::FEBRUARY,days(7),hours(5),minutes(0),seconds(0),milliseconds(0)) );
    // + days (overflow)
    time += days(375+(time.leap_year()?1:0));
    BOOST_CHECK( time == DateTime(2017,Month::FEBRUARY,days(17),hours(5),minutes(0),seconds(0),milliseconds(0)) );
    // + months (no overflow)
    time += days(28+31+30);
    BOOST_CHECK( time == DateTime(2017,Month::MAY,days(17),hours(5),minutes(0),seconds(0),milliseconds(0)) );

    // compound setters
    time.set_time(DateTime(1976, Month::MAY, days(21), hours(18), minutes(35), seconds(37), milliseconds(97)));
    BOOST_CHECK( time == DateTime(2017, Month::MAY, days(17), hours(18), minutes(35), seconds(37), milliseconds(97)) );
    time.set_time(hours(12),minutes(34),seconds(56),milliseconds(78));
    BOOST_CHECK( time == DateTime(2017,Month::MAY,days(17),hours(12),minutes(34),seconds(56),milliseconds(78)) );
    time.set_date(1974, Month::JULY, days(25));
    BOOST_CHECK( time == DateTime(1974,Month::JULY,days(25),hours(12),minutes(34),seconds(56),milliseconds(78)) );
    time.set_date(Month::JUNE, days(26));
    BOOST_CHECK( time == DateTime(1974,Month::JUNE,days(26),hours(12),minutes(34),seconds(56),milliseconds(78)) );
    time.set_date(DateTime(1976, Month::MAY, days(21)));
    BOOST_CHECK( time == DateTime(1976,Month::MAY,days(21),hours(12),minutes(34),seconds(56),milliseconds(78)) );
    time.set_month(Month::APRIL);
    BOOST_CHECK( time == DateTime(1976,Month::APRIL,days(21),hours(12),minutes(34),seconds(56),milliseconds(78)) );


    time = DateTime(2015,Month::JANUARY,days(1),hours(0),minutes(0));
    // - milliseconds (full underflow)
    time -= milliseconds(500);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(31),hours(23),minutes(59),seconds(59),milliseconds(500)) );
    // - milliseconds (no underflow)
    time -= milliseconds(500);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(31),hours(23),minutes(59),seconds(59),milliseconds(0)) );
    // - seconds (no underflow)
    time -= seconds(50);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(31),hours(23),minutes(59),seconds(9)) );
    // - seconds (underflow)
    time -= seconds(69);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(31),hours(23),minutes(58)) );
    // - minutes (no underflow)
    time -= minutes(50);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(31),hours(23),minutes(8)) );
    // - minutes (underflow)
    time -= minutes(8+60*2);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(31),hours(21),minutes(0)) );
    // - hours (no underflow)
    time -= hours(11);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(31),hours(10),minutes(0)) );
    // - hours (underflow)
    time -= hours(34);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(30),hours(0),minutes(0)) );
    // - days (no underflow)
    time -= days(20);
    BOOST_CHECK( time == DateTime(2014,Month::DECEMBER,days(10),hours(0),minutes(0)) );
    // - days (underflow)
    time -= days(20);
    BOOST_CHECK( time == DateTime(2014,Month::NOVEMBER,days(20),hours(0),minutes(0)) );
    // - months (no underflow)
    time -= days(30+31+30);
    BOOST_CHECK( time == DateTime(2014,Month::AUGUST,days(20),hours(0),minutes(0)) );
    // - days (underflow)
    time -= days(time.year_day());
    BOOST_CHECK( time == DateTime(2014,Month::JANUARY,days(1),hours(0),minutes(0)) );
    // - year
    time -= days(365);
    BOOST_CHECK( time == DateTime(2013,Month::JANUARY,days(1),hours(0),minutes(0)) );
    // - years
    time -= days(366+365);
    BOOST_CHECK( time == DateTime(2011,Month::JANUARY,days(1),hours(0),minutes(0)) );

    // comparison
    BOOST_CHECK( time+seconds(1) < time+milliseconds(1001) );
    BOOST_CHECK( time < time+milliseconds(1) );
    BOOST_CHECK( !(time+milliseconds(1) < time) );
    BOOST_CHECK( time <= time+milliseconds(1) );
    BOOST_CHECK( !(time+milliseconds(1) <= time) );

    BOOST_CHECK( !(time > time+milliseconds(1)) );
    BOOST_CHECK( time+milliseconds(1) > time );
    BOOST_CHECK( !(time >= time+milliseconds(1)) );
    BOOST_CHECK( time+milliseconds(1) >= time );

    BOOST_CHECK( time != time+milliseconds(1) );
    BOOST_CHECK( time+milliseconds(1) != time );
    BOOST_CHECK( !(time != time) );

    // year_day
    for ( int i = 0; i < 31; i++ )
        BOOST_CHECK( DateTime(2013,Month::JANUARY,days(1+i),hours(0),minutes(0)).year_day() == i );

    for ( int i = 0; i < 30; i++ )
        BOOST_CHECK( DateTime(2013,Month::MARCH,days(1+i),hours(0),minutes(0)).year_day() == i+28+31 );

    // difference
    BOOST_CHECK( time-time == DateTime::Clock::duration::zero() );
    auto cmp1 = [&time](auto duration) {
        return (time+duration) - time == duration;
    };
    auto cmp2 = [&time](auto duration) {
        return time - (time+duration) ==
            -std::chrono::duration_cast<DateTime::Clock::duration>(duration);
    };
    BOOST_CHECK( cmp1(milliseconds(45)) );
    BOOST_CHECK( cmp2(milliseconds(45)) );
    BOOST_CHECK( cmp1(seconds(45)) );
    BOOST_CHECK( cmp2(seconds(45)) );
    BOOST_CHECK( cmp1(minutes(45)) );
    BOOST_CHECK( cmp2(minutes(45)) );
    BOOST_CHECK( cmp1(hours(45)) );
    BOOST_CHECK( cmp2(hours(45)) );
    BOOST_CHECK( cmp1(days(45)) );
    BOOST_CHECK( cmp2(days(45)) );

    // unix
    DateTime unix(1970,Month::JANUARY,days(1),hours(0),minutes(0));
    BOOST_CHECK( unix.unix() == 0 );
    BOOST_CHECK( (unix+milliseconds(12)).unix() == 0 );
    BOOST_CHECK( (unix+seconds(1)).unix() == 1 );
    BOOST_CHECK( (unix+minutes(2)).unix() == 2*60 );
    BOOST_CHECK( (unix+hours(3)).unix() == 3*60*60 );
    BOOST_CHECK( (unix+days(4)).unix() == 4*24*60*60 );
    BOOST_CHECK( DateTime(2015,Month::MARCH,days(28),hours(18),minutes(53),seconds(30)).unix() == 1427568810  );
    BOOST_CHECK( DateTime(1915,Month::MARCH,days(28),hours(18),minutes(53),seconds(30)).unix() == -1728191190 );

    // time_point()
    BOOST_CHECK( unix.time_point().time_since_epoch().count() == 0 );
    auto time_point_check = [](const DateTime& dt, int64_t millistamp)
    {
        return std::chrono::duration_cast<milliseconds>(
            dt.time_point().time_since_epoch()).count() == millistamp;
    };
    BOOST_CHECK( time_point_check(
        {2015,Month::MARCH,days(28),hours(18),minutes(53),seconds(30),milliseconds(123)},
        1427568810123 ) );
    BOOST_CHECK( time_point_check(
        {1915,Month::MARCH,days(28),hours(18),minutes(53),seconds(30),milliseconds(1)},
        -1728191189999 ) );

    // default ctor (calls Clock::time_point ctor)
    BOOST_CHECK( std::abs( DateTime().unix() - std::time(nullptr) ) < 1);

    // week_day
    auto week_day_check = [](int y, Month m, days day, WeekDay wday)
    {
        return DateTime(y,m,day,hours(0),minutes(0)).week_day() == wday;
    };
    BOOST_CHECK( week_day_check(2015,Month::MARCH,days(29),WeekDay::SUNDAY) );
    BOOST_CHECK( week_day_check(2015,Month::MARCH,days(1),WeekDay::SUNDAY) );
    BOOST_CHECK( week_day_check(2000,Month::MARCH,days(1),WeekDay::WEDNESDAY) );
    BOOST_CHECK( week_day_check(1582,Month::OCTOBER,days(15),WeekDay::FRIDAY) );
    // Still gregorian before 1582-10-15
    BOOST_CHECK( week_day_check(1582,Month::OCTOBER,days(1),WeekDay::FRIDAY) );
    BOOST_CHECK( week_day_check(1,Month::DECEMBER,days(25),WeekDay::TUESDAY) );
    BOOST_CHECK( week_day_check(0,Month::DECEMBER,days(31),WeekDay::SUNDAY) );
    BOOST_CHECK( week_day_check(-1,Month::DECEMBER,days(31),WeekDay::SUNDAY) );
    BOOST_CHECK( week_day_check(-10,Month::JANUARY,days(1),WeekDay::TUESDAY) );
    BOOST_CHECK( week_day_check(-44,Month::MARCH,days(15),WeekDay::FRIDAY) );

}


BOOST_AUTO_TEST_CASE( test_MonthWeek_string )
{
    BOOST_CHECK( month_name(Month::APRIL) == "April" );
    BOOST_CHECK( month_name(Month(13)) == "" );

    BOOST_CHECK( month_shortname(Month::APRIL) == "Apr" );
    BOOST_CHECK( month_shortname(Month(13)) == "" );

    BOOST_CHECK( *month_from_name("Apr") == Month::APRIL );
    BOOST_CHECK( *month_from_name("April") == Month::APRIL );
    BOOST_CHECK( *month_from_name("APRIL") == Month::APRIL );
    BOOST_CHECK( !month_from_name("APRUL") );


    BOOST_CHECK( weekday_name(WeekDay::FRIDAY) == "Friday" );
    BOOST_CHECK( weekday_name(WeekDay(13)) == "" );

    BOOST_CHECK( weekday_shortname(WeekDay::FRIDAY) == "Fri" );
    BOOST_CHECK( weekday_shortname(WeekDay(13)) == "" );

    BOOST_CHECK( *weekday_from_name("Fri") == WeekDay::FRIDAY );
    BOOST_CHECK( *weekday_from_name("Friday") == WeekDay::FRIDAY );
    BOOST_CHECK( *weekday_from_name("FRIDAY") == WeekDay::FRIDAY );
    BOOST_CHECK( !weekday_from_name("FREEDAY") );

}

BOOST_AUTO_TEST_CASE( test_format )
{
    DateTime time(2015,Month::APRIL,days(4),hours(15),minutes(0),seconds(0),milliseconds(5));
    DateTime time2(2015,Month::APRIL,days(5),hours(8),minutes(0),seconds(0),milliseconds(5));
    // day
    BOOST_CHECK( format_char(time,'d') == "04" );
    BOOST_CHECK( format_char(time,'D') == "Sat" );
    BOOST_CHECK( format_char(time,'j') == "4" );
    BOOST_CHECK( format_char(time,'l') == "Saturday" );
    BOOST_CHECK( format_char(time,'N') == "6" );
    BOOST_CHECK( format_char(time2,'N') == "7" );
    BOOST_CHECK( format_char(time,'S') == "th" );
    BOOST_CHECK( format_char(time,'w') == "6" );
    BOOST_CHECK( format_char(time2,'w') == "0" );
    BOOST_CHECK( format_char(time,'z') == "93" );
    // week
    //BOOST_CHECK( format_char(time,'W') == "14" );
    // month
    BOOST_CHECK( format_char(time,'F') == "April" );
    BOOST_CHECK( format_char(time,'m') == "04" );
    BOOST_CHECK( format_char(time,'M') == "Apr" );
    BOOST_CHECK( format_char(time,'n') == "4" );
    BOOST_CHECK( format_char(time,'t') == "30" );
    // year
    BOOST_CHECK( format_char(time,'L') == "0" );
    //BOOST_CHECK( format_char(time,'o') == "2015" );
    BOOST_CHECK( format_char(time,'Y') == "2015" );
    BOOST_CHECK( format_char(time,'y') == "15" );
    // time
    BOOST_CHECK( format_char(time,'a') == "pm" );
    BOOST_CHECK( format_char(time,'A') == "PM" );
    //BOOST_CHECK( format_char(time,'B') == "625" );
    BOOST_CHECK( format_char(time,'g') == "3" );
    BOOST_CHECK( format_char(time,'G') == "15" );
    BOOST_CHECK( format_char(time2,'G') == "8" );
    BOOST_CHECK( format_char(time,'h') == "03" );
    BOOST_CHECK( format_char(time,'H') == "15" );
    BOOST_CHECK( format_char(time2,'H') == "08" );
    BOOST_CHECK( format_char(time,'i') == "00" );
    BOOST_CHECK( format_char(time,'s') == "00" );
    BOOST_CHECK( format_char(time,'u') == "005000" );
    // todo timezone
    // full date time
    BOOST_CHECK( format_char(time,'c') == "2015-04-04T15:00:00" );
    BOOST_CHECK( format_char(time,'r') == "Sat, 04 Apr 2015 15:00:00" );
    BOOST_CHECK( format_char(time,'U') == "1428159600" );


    // Custom formats
    BOOST_CHECK( format(time, "Y-m-d H:i:s.u") == "2015-04-04 15:00:00.005000" );
    BOOST_CHECK( format(time, "l, F \\t\\h\\e jS, g:i a") == "Saturday, April the 4th, 3:00 pm" );
    BOOST_CHECK( strftime(time, "%l, %F the %j%S, %g:%i %a %%") == "Saturday, April the 4th, 3:00 pm %" );

    DateTime dt;
    BOOST_CHECK( format(dt, "Y-m-d") == format("Y-m-d") );
    BOOST_CHECK( strftime(dt, "%Y-%m-%d") == strftime("%Y-%m-%d") );
}

BOOST_AUTO_TEST_CASE( test_TimeParser )
{
    // ATOMIC_DURATION
    BOOST_CHECK( parse_duration("6 ponies") == DateTime::Duration::zero() );

    BOOST_CHECK( parse_duration("123ms") == milliseconds(123) );
    BOOST_CHECK( parse_duration("123 millisecond") == milliseconds(123) );
    BOOST_CHECK( parse_duration("123 milliseconds") == milliseconds(123) );

    BOOST_CHECK( parse_duration("123s") == seconds(123) );
    BOOST_CHECK( parse_duration("123\"") == seconds(123) );
    BOOST_CHECK( parse_duration("123 second") == seconds(123) );
    BOOST_CHECK( parse_duration("123 seconds") == seconds(123) );

    BOOST_CHECK( parse_duration("123m") == minutes(123) );
    BOOST_CHECK( parse_duration("123min") == minutes(123) );
    BOOST_CHECK( parse_duration("123'") == minutes(123) );
    BOOST_CHECK( parse_duration("123 minute") == minutes(123) );
    BOOST_CHECK( parse_duration("123 minutes") == minutes(123) );

    BOOST_CHECK( parse_duration("123h") == hours(123) );
    BOOST_CHECK( parse_duration("123 hour") == hours(123) );
    BOOST_CHECK( parse_duration("123 hours") == hours(123) );

    BOOST_CHECK( parse_duration("123d") == days(123) );
    BOOST_CHECK( parse_duration("123 day") == days(123) );
    BOOST_CHECK( parse_duration("123 days") == days(123) );

    BOOST_CHECK( parse_duration("123w") == weeks(123) );
    BOOST_CHECK( parse_duration("123 week") == weeks(123) );
    BOOST_CHECK( parse_duration("123 weeks") == weeks(123) );

    // HOUR_OPT
    BOOST_CHECK( parse_duration("43:21") == hours(43)+minutes(21) );
    BOOST_CHECK( parse_duration("43:21:23") == hours(43)+minutes(21)+seconds(23) );
    BOOST_CHECK( parse_duration("43:21:23.456") == hours(43)+minutes(21)+seconds(23)+milliseconds(456) );

    BOOST_CHECK( parse_duration("43:21 h") == hours(43)+minutes(21) );
    BOOST_CHECK( parse_duration("43:21:23 h") == hours(43)+minutes(21)+seconds(23) );
    BOOST_CHECK( parse_duration("43:21:23.456 h") == hours(43)+minutes(21)+seconds(23)+milliseconds(456) );
    BOOST_CHECK( parse_duration("PT43:21:23") == hours(43)+minutes(21)+seconds(23) );

    BOOST_CHECK( parse_duration("21:23 m") == minutes(21)+seconds(23) );
    /// \todo allow "21:23.456", which must be minutes

    // DURATION_SEQ
    BOOST_CHECK( parse_duration("1 day 2 hours 3 minutes") == days(1)+hours(2)+minutes(3) );

    // DURATION
    BOOST_CHECK( parse_duration("1:23 hours 56 seconds") == hours(1)+minutes(23)+seconds(56) );

    // NOW_TIME
    auto nowcheck = [](const DateTime& a, const DateTime& b)
    {
        auto delta = std::max(a,b) - std::min(a,b);
        return delta < seconds(1);
    };
    DateTime now;
    BOOST_CHECK( nowcheck(parse_time("now"),now) );
    BOOST_CHECK( nowcheck(parse_time("now + 3 hours"),now+hours(3)) );
    BOOST_CHECK( nowcheck(parse_time("now - 3 hours"),now-hours(3)) );
    BOOST_CHECK( nowcheck(parse_time("in 3 hours"),now+hours(3)) );

    // at
    DateTime at;
    at.set_time(hours(17), minutes(0));
    BOOST_CHECK( nowcheck(parse_time("at 5 pm"), at) );
    BOOST_CHECK( nowcheck(parse_time("at 17"), at) );
    BOOST_CHECK( nowcheck(parse_time("at 17:00"), at) );

    // DAY
    BOOST_CHECK( parse_time("2015-04-03 00:00") == DateTime(2015,Month::APRIL,days(03)) );
    now = DateTime();
    BOOST_CHECK( nowcheck(parse_time("tomorrow"),now+days(1)) );
    BOOST_CHECK( nowcheck(parse_time("yesterday"),now-days(1)) );
    BOOST_CHECK( nowcheck(parse_time("today"),now) );

    // DATE_DESC
    BOOST_CHECK( parse_time("April 4 00:00") == DateTime(now.year(),Month::APRIL,days(04)) );
    BOOST_CHECK( parse_time("April 4 2015 00:00") == DateTime(2015,Month::APRIL,days(04)) );
    BOOST_CHECK( parse_time("April 4th 2015 00:00") == DateTime(2015,Month::APRIL,days(04)) );
    BOOST_CHECK( parse_time("Saturday, April 4th 2015 00:00") == DateTime(2015,Month::APRIL,days(04)) );
    BOOST_CHECK( parse_time("4th April 2015 00:00") == DateTime(2015,Month::APRIL,days(04)) );
    DateTime monday;
    do
        monday += days(1);
    while ( monday.week_day() != WeekDay::MONDAY );
    BOOST_CHECK( nowcheck(parse_time("Monday"),monday) );
    BOOST_CHECK( nowcheck(parse_time("next Monday"),monday) );

    // TIME
    DateTime check;
    check.set_time(hours(12),minutes(34));
    BOOST_CHECK( parse_time("12:34") == check );

    BOOST_CHECK( parse_time("2015-04-04 14:50") == DateTime(2015,Month::APRIL,days(04),hours(14),minutes(50)) );
    BOOST_CHECK( parse_time("2015-04-04 02:50 pm") == DateTime(2015,Month::APRIL,days(04),hours(14),minutes(50)) );
    BOOST_CHECK( parse_time("2015-04-04 02:50 am") == DateTime(2015,Month::APRIL,days(04),hours(02),minutes(50)) );
    BOOST_CHECK( parse_time("2015-04-04 at 02:50 pm") == DateTime(2015,Month::APRIL,days(04),hours(14),minutes(50)) );
    BOOST_CHECK( parse_time("2015-04-04 at 2 pm") == DateTime(2015,Month::APRIL,days(04),hours(14),minutes(00)) );
    BOOST_CHECK( parse_time("2015-04-04 12:00 am") == DateTime(2015,Month::APRIL,days(04),hours(00),minutes(00)) );
    BOOST_CHECK( parse_time("2015-04-04 12:00 pm") == DateTime(2015,Month::APRIL,days(04),hours(12),minutes(00)) );

    // DATE_TIME
    BOOST_CHECK( parse_time("2015-04-04T15:00") == DateTime(2015,Month::APRIL,days(04),hours(15),minutes(00)) );
    BOOST_CHECK( parse_time("2015-04-04 at 15:00") == DateTime(2015,Month::APRIL,days(04),hours(15),minutes(00)) );

    // Failures
    now = DateTime();
    BOOST_CHECK( nowcheck(parse_time("?"),now) );
    BOOST_CHECK( nowcheck(parse_time("2015-05"),now) );
    BOOST_CHECK( nowcheck(parse_time("2015-13-01"),now) );

    // get_remainder
    std::stringstream stream("2015-04-04 14:50 lol");
    TimeParser parser(stream);
    BOOST_CHECK( parser.parse_time_point() == DateTime(2015,Month::APRIL,days(04),hours(14),minutes(50)) );
    BOOST_CHECK( parser.get_remainder() == "lol" );
    BOOST_CHECK( parser.get_remainder().empty() );
}

BOOST_AUTO_TEST_CASE( test_duration_string )
{
    BOOST_CHECK( duration_string(days(9)) == "1 week 2 days 0 hours 0 minutes" );
    BOOST_CHECK( duration_string(seconds(69)) == "1 minute 9 seconds" );
}
