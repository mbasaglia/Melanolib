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
#ifndef MELANOLIB_HASH_HPP
#define MELANOLIB_HASH_HPP

#include <functional>
#include <type_traits>

namespace melanolib {

inline constexpr std::size_t hash_combine(std::size_t a, std::size_t b)
{
    return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
}

template<class Container, class ElementHasher=std::hash<typename Container::value_type>>
struct ContainerHasher
{
    std::size_t operator()(const Container& container) const
    {
        std::size_t hash = 0;
        ElementHasher hasher;
        for ( const auto& item : container )
            hash = hash_combine(hash, hasher(item));
        return hash;
    }
};

template<class T>
using Hasher = std::hash<std::decay_t<T>>;

template<class T>
std::size_t hash(T&& value)
{
    return Hasher<T>()(value);
}

constexpr std::size_t multi_hash()
{
    return 0;
}

template<class Head, class... Args>
inline constexpr std::size_t multi_hash(Head&& head, Args&&... args)
{
    return hash_combine(
        hash(std::forward<Head>(head)),
        multi_hash(std::forward<Args>(args)...)
    );
}


} // namespace melanolib
#endif // MELANOLIB_HASH_HPP
