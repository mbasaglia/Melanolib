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
#ifndef MELANOLIB_MATH_VECTOR_HPP
#define MELANOLIB_MATH_VECTOR_HPP

#include <iterator>

namespace melanolib {
namespace math {

/**
 * \note On C++17 all the container stuff can be removed by inheriting std::array
 */
template<class T, std::size_t Size>
    class Vector
{
public:
    using value_type             = T;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    value_type _arr[Size] = {0};

    constexpr reference operator[](size_type i)
    {
        return _arr[i];
    }
    constexpr const_reference operator[](size_type i) const
    {
        return _arr[i];
    }

    constexpr pointer data()
    {
        return _arr;
    }
    constexpr const_pointer data() const
    {
        return _arr;
    }

    constexpr iterator begin()
    {
        return _arr;
    }
    constexpr const_iterator begin() const
    {
        return _arr;
    }
    constexpr const_iterator cbegin() const
    {
        return _arr;
    }

    constexpr iterator end()
    {
        return _arr + Size;
    }
    constexpr const_iterator end() const
    {
        return _arr + Size;
    }
    constexpr const_iterator cend() const
    {
        return _arr + Size;
    }

    constexpr reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(end());
    }

    constexpr reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }
    constexpr const_reverse_iterator crend() const
    {
        return const_reverse_iterator(begin());
    }

    constexpr bool empty() const
    {
        return !Size;
    }
    constexpr size_type size() const
    {
        return Size;
    }
    constexpr size_type max_size() const
    {
        return Size;
    }

    constexpr Vector& operator+=(const Vector& oth)
    {
        for ( size_type i = 0; i < Size; i++ )
            _arr[i] += oth[i];
        return *this;
    }

    constexpr Vector operator+(const Vector& oth) const
    {
        return Vector(*this) += oth;
    }

    constexpr Vector& operator-=(const Vector& oth)
    {
        for ( size_type i = 0; i < Size; i++ )
            _arr[i] -= oth[i];
        return *this;
    }

    constexpr Vector operator-(const Vector& oth) const
    {
        return Vector(*this) -= oth;
    }

    friend constexpr Vector operator+(const Vector& v)
    {
        return v;
    }

    friend constexpr Vector operator-(Vector v)
    {
        for ( auto& e : v )
            e = -e;
        return v;
    }

    constexpr Vector& operator*=(value_type scalar)
    {
        for ( auto& e : _arr )
            e *= scalar;
        return *this;
    }

    constexpr Vector operator*(value_type scalar) const
    {
        return Vector(*this) *= scalar;
    }

    friend constexpr Vector operator*(value_type scalar, Vector v)
    {
        return v * scalar;
    }

    constexpr Vector& operator/=(value_type scalar)
    {
        for ( auto& e : _arr )
            e /= scalar;
        return *this;
    }

    constexpr Vector operator/(value_type scalar) const
    {
        return Vector(*this) /= scalar;
    }

    constexpr bool operator==(const Vector& oth) const
    {
        for ( size_type i = 0; i < Size; i++ )
            if ( _arr[i] != oth[i] )
                return false;
        return true;
    }

    constexpr bool operator!=(const Vector& oth) const
    {
        return !(*this == oth);
    }

    constexpr bool operator<(const Vector& oth) const
    {
        for ( size_type i = 0; i < Size; i++ )
            if ( _arr[i] < oth[i] )
                return true;
            else if ( _arr[i] > oth[i] )
                return false;
        return _arr[Size-1] != oth[Size-1];
    }

    constexpr bool operator<=(const Vector& oth) const
    {
        for ( size_type i = 0; i < Size; i++ )
            if ( _arr[i] < oth[i] )
                return true;
            else if ( _arr[i] > oth[i] )
                return false;
        return true;
    }

    constexpr bool operator>(const Vector& oth) const
    {
        for ( size_type i = 0; i < Size; i++ )
            if ( _arr[i] < oth[i] )
                return false;
            else if ( _arr[i] > oth[i] )
                return true;
        return _arr[Size-1] != oth[Size-1];
    }

    constexpr bool operator>=(const Vector& oth) const
    {
        for ( size_type i = 0; i < Size; i++ )
            if ( _arr[i] < oth[i] )
                return false;
            else if ( _arr[i] > oth[i] )
                return true;
        return true;
    }
};

template<class T>
    using Vec3 = Vector<T, 3>;

} // namespace math
} // namespace melanolib
#endif // MELANOLIB_MATH_VECTOR_HPP
