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
#ifndef MELANOLIB_GSL_HPP
#define MELANOLIB_GSL_HPP

#include <memory>
#include <array>
#include <vector>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <type_traits>
#include "c++-compat.hpp"


namespace melanolib {

/**
 * \brief A simple GSL implementation for C++11 (and C++14)
 * \see https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#gsl-guideline-support-library
 */
namespace gsl {

template<class T>
    using owner = T;

template<class T>
    constexpr owner<T> move_owner(owner<T>&& owner)
    {
        return std::move(owner);
    }

using zstring = char*;
using czstring = const char*;

using std::unique_ptr;
using std::shared_ptr;

template<class T, std::size_t size>
    using stack_array = std::array<T, size>;

/// \todo
template<class T>
    using dyn_array = std::vector<T>;


/// \todo Macros to disable expression evaluation
inline void Expects(bool expression, czstring message = nullptr)
{
    if ( !expression )
        throw std::logic_error(message ? message : "");
}

inline void Ensures(bool expression, czstring message = nullptr)
{
    if ( !expression )
        throw std::logic_error(message ? message : "");
}

template<class T>
    class not_null
{
public:
    SUPER_CONSTEXPR explicit not_null(const T& ptr)
        : ptr(ptr)
    {
        Expects(this->ptr != nullptr);
    }
    SUPER_CONSTEXPR explicit not_null(T&& ptr)
        : ptr(std::move(ptr))
    {
        Expects(this->ptr != nullptr);
    }
    not_null(const std::nullptr_t&) = delete;
    not_null(int) = delete;
    not_null(const not_null &other) = default;
    not_null(not_null &&other) = default;

    SUPER_CONSTEXPR not_null& operator=(const T& ptr)
    {
        Expects(ptr != nullptr);
        this->ptr = ptr;
        return *this;
    }
    SUPER_CONSTEXPR not_null& operator=(T&& ptr)
    {
        Expects(ptr != nullptr);
        this->ptr = std::move(ptr);
        return *this;
    }
    not_null& operator=(const std::nullptr_t&) = delete;
    not_null& operator=(int) = delete;
    SUPER_CONSTEXPR not_null& operator=(const not_null &other) = default;
    SUPER_CONSTEXPR not_null& operator=(not_null &&other) = default;

    SUPER_CONSTEXPR operator T() const { return ptr; }
    SUPER_CONSTEXPR T operator->() const { return ptr; }
    SUPER_CONSTEXPR decltype(*std::declval<T>()) operator*() const { return *ptr; }

    SUPER_CONSTEXPR bool operator==(const T& rhs) const { return ptr == rhs; }
    SUPER_CONSTEXPR bool operator!=(const T& rhs) const { return !(*this == rhs); }

private:
    T ptr;
};

template<class T>
    class array_view
{
public:
    using value_type = T;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;
    using size_type = std::size_t;

    SUPER_CONSTEXPR array_view(pointer begin, pointer end)
        : begin_(begin), end_(end)
    {
        Expects(begin <= end);
    }

    SUPER_CONSTEXPR array_view(pointer begin, size_type size)
        : begin_(begin), end_(begin+size)
    {
        Expects(size == 0 || (size > 0 && begin != nullptr));
    }

    constexpr array_view()
        : begin_(nullptr), end_(nullptr)
    {
    }

    template<std::size_t size>
        SUPER_CONSTEXPR array_view(T (&array)[size])
        :  array_view(array, size)
    {}

    template<class Iterator, class =
        typename std::enable_if<
            std::is_same<typename Iterator::value_type, value_type>::value
        >::type>
    array_view(const Iterator& begin, const Iterator& end)
        : array_view(&*begin, &*end)
    {
        Expects(std::distance(begin, end) == &*end - &*begin);
    }

    template<class Container, class =
        typename std::enable_if<
            std::is_same<typename Container::value_type, value_type>::value
        >::type>
    array_view(Container& container)
        : array_view(container.data(), container.size())
    {
    }

    constexpr size_type size() const
    {
        return end_ - begin_;
    }

    constexpr bool empty() const
    {
        return end_ == begin_;
    }

    SUPER_CONSTEXPR reference operator[] (size_type i) const
    {
        Expects(i >= 0 && i < size());
        return begin_[i];
    }

    constexpr explicit operator bool() const
    {
        return begin_;
    }

    constexpr iterator begin() const
    {
        return begin_;
    }

    constexpr iterator end() const
    {
        return end_;
    }

    constexpr const_iterator cbegin() const
    {
        return begin_;
    }

    constexpr const_iterator cend() const
    {
        return end_;
    }

    constexpr const_iterator rbegin() const
    {
        return reverse_iterator(end_);
    }

    constexpr const_iterator rend() const
    {
        return reverse_iterator(begin_);
    }

    constexpr const_reverse_iterator crbegin() const
    {
        return reverse_iterator(cbegin());
    }

    constexpr const_reverse_iterator crend() const
    {
        return reverse_iterator(cbegin());
    }

    constexpr bool operator== (const array_view& rhs) const
    {
        return begin_ == rhs.begin_ && end_ == rhs.end_;
    }

    constexpr bool operator!= (const array_view& rhs) const
    {
        return !(*this == rhs);
    }

private:
    T* begin_;
    T* end_;
};

/// \todo
template<class T>
    class array_view_p;

using string_view = array_view<char>;
using cstring_view = array_view<const char>;

template<class Functor>
    class Final_act
{
public:
    explicit Final_act(const Functor& func) : func(func) {}
    ~Final_act() { func(); }

private:
    Functor func;
};

template<class Functor>
    inline Final_act<Functor> finally(const Functor& func)
    {
        return Final_act<Functor>(func);
    }

template<class T, class U>
    T narrow_cast(U value)
    {
        return static_cast<T>(value);
    }

class narrowing_error : public std::exception {};

template<class T, class U>
    T narrow(U value)
    {
        auto narrowed = narrow_cast<T>(value);
        if ( !(narrowed == value) )
            throw narrowing_error{};
        return narrowed;
    }

#define implicit

} // namespace gsl
} // namespace melanolib
#endif // MELANOLIB_GSL_HPP
