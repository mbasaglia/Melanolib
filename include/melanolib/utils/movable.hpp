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
#ifndef MELANOLIB_UTILS_MOVABLE_HPP
#define MELANOLIB_UTILS_MOVABLE_HPP

#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {

/**
 * \brief Makes a non-copiable, non-movable type movable
 */
template<class Contained>
class Movable
{
public:
    template<class... Args>
        Movable(Args&&... args)
            : data(New<Contained>(std::forward<Args>(args)...))
        {}

    operator Contained&()
    {
        return *data;
    }

    operator const Contained&() const
    {
        return *data;
    }

    Contained& operator*()
    {
        return *data;
    }

    const Contained& operator*() const
    {
        return *data;
    }

    Contained* operator->()
    {
        return data.get();
    }

    const Contained* operator->() const
    {
        return data.get();
    }

private:
    std::unique_ptr<Contained> data;
};

} // namespace melanolib
#endif // MELANOLIB_UTILS_MOVABLE_HPP
