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
#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#include <functional>
#include <utility>
#include "melanolib/utils/type_utils.hpp"

namespace melanolib {

/**
 * \brief Call a std::function when it is properly initialized
 * \tparam Functor  Callable which can be converted to bool
 * \tparam CallArgs Argument types used at the call point
 * \param  function Function object
 * \param  args     Function arguments
 */
template<class Functor, class... CallArgs>
    std::enable_if_t<ExplicitlyConvertible<Functor, bool>::value>
    callback(const Functor& function, CallArgs&&... args)
    {
        if ( function )
            function(std::forward<CallArgs>(args)...);
    }

template<class Functor, class... CallArgs>
    std::enable_if_t<!ExplicitlyConvertible<Functor, bool>::value>
    callback(const Functor& function, CallArgs&&... args)
    {
        function(std::forward<CallArgs>(args)...);
    }

/**
 * \brief Utility to call a function on a range
 */
template<class Functor, class Range, class... Args>
    auto range_call(const Functor& functor, Range& range, Args&&... args)
    {
        return functor(std::begin(range), std::end(range), std::forward<Args>(args)...);
    }

template<class Functor, class Range, class... Args>
    auto range_call(const Functor& functor, const Range& range, Args&&... args)
    {
        return functor(std::begin(range), std::end(range), std::forward<Args>(args)...);
    }

namespace detail {

    template<class Range>
        using IteratorType = decltype(std::begin(std::declval<Range>()));

    template<class Return, class Iterator, class... Args>
        using ResolvedRangeOverload = FunctionPointer<Return (Iterator, Iterator, Args...)>;
}

template<class Return, class Range, class... Args>
    auto range_call_overload(
        detail::ResolvedRangeOverload<Return, detail::IteratorType<Range>, Args...> functor,
        Range&& range, Args&&... args)
    {
        return range_call(functor, std::forward<Range>(range), std::forward<Args>(args)...);
    }

struct Noop
{
    constexpr Noop(){}

    template<class... Args>
        constexpr void operator()(Args&&... args) const
        {
        }

    constexpr explicit operator bool() const
    {
        return false;
    }
    constexpr bool operator!() const
    {
        return true;
    }
};

/**
 * \brief Identity function object
 */
struct Identity
{
    template<class T>
    decltype(auto) operator()(T&& t) const
    {
        return std::forward<T>(t);
    }
};

/**
 * \brief Function object returning the begin iterator
 */
struct Begin
{
    template<class T>
    decltype(auto) operator()(T&& t) const
    {
        return std::begin(std::forward<T>(t));
    }
};

/**
 * \brief Function object returning the begin iterator
 */
struct End
{
    template<class T>
    decltype(auto) operator()(T&& t) const
    {
        return std::end(std::forward<T>(t));
    }
};

} // namespace melanolib
#endif // FUNCTIONAL_HPP
