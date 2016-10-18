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
#ifndef MELANOLIB_UTILS_DESCRIBE_TYPE_HPP
#define MELANOLIB_UTILS_DESCRIBE_TYPE_HPP

#include <typeinfo>
#include <string>
#include "c++-compat.hpp"

namespace melanolib {

namespace detail {

    template<class T> struct MemberData
    {
        using class_type = void;
        using member_type = T;
    };

    template<class Class, class Ret, class... Args>
    struct MemberData<Ret (Class::*) (Args...)>
    {
        using class_type = Class;
        using member_type = Ret;
    };

    template<class Class, class Ret>
    struct MemberData<Ret Class::*>
    {
        using class_type = Class;
        using member_type = Ret;
    };


    template<class T>
    std::enable_if_t<
        !std::is_member_function_pointer<T>::value &&
        !std::is_function<T>::value,
        const char*>
    get_name() { return typeid(T).name(); }

    template<class T>
    std::enable_if_t<
        std::is_member_function_pointer<T>::value ||
        std::is_function<T>::value,
        const char*>
    get_name() { return "error"; }

} // namespace detail

template<class T>
std::string describe_type()
{
    if CONSTEXPR_IF ( std::is_void<T>::value )
        return "void";
    if CONSTEXPR_IF ( std::is_rvalue_reference<T>::value )
        return "rvalue reference to " + describe_type<std::remove_reference_t<T>>();
    if CONSTEXPR_IF ( std::is_lvalue_reference<T>::value )
        return "reference to " + describe_type<std::remove_reference_t<T>>();
    if CONSTEXPR_IF ( std::is_array<T>::value )
    {
        std::string desc;
        if ( std::rank<T>::value > 1 )
            desc = std::to_string(std::rank<T>::value) + "-dimensional ";
        return desc + "array of " + describe_type<std::remove_all_extents_t<T>>();
    }
    if CONSTEXPR_IF ( std::is_const<T>::value )
        return "const " + describe_type<std::remove_const_t<T>>();
    if CONSTEXPR_IF ( std::is_volatile<T>::value )
        return "volatile " + describe_type<std::remove_volatile_t<T>>();
    if CONSTEXPR_IF ( std::is_member_function_pointer<T>::value )
    {
        using Data = detail::MemberData<T>;
        return "pointer to member function of " +
            describe_type<typename Data::class_type>();
    }
    if CONSTEXPR_IF ( std::is_member_object_pointer<T>::value )
    {
        using Data = detail::MemberData<T>;
        return "pointer to member of " +
            describe_type<typename Data::class_type>() +
            " of type " + describe_type<typename Data::member_type>();
    }
    if CONSTEXPR_IF ( std::is_pointer<T>::value )
        return "pointer to " + describe_type<std::remove_pointer_t<T>>();

    std::string desc;

    if CONSTEXPR_IF ( std::is_final<T>::value )
        desc += "final ";
    else if CONSTEXPR_IF ( std::is_abstract<T>::value )
        desc += "abstract ";
    else if CONSTEXPR_IF ( std::is_polymorphic<T>::value )
        desc += "polymorphic ";

    if CONSTEXPR_IF ( std::is_class<T>::value )
        desc += "class ";
    else if CONSTEXPR_IF ( std::is_enum<T>::value )
        desc += "enum ";
    else if CONSTEXPR_IF ( std::is_union<T>::value )
        desc += "union ";
    else if CONSTEXPR_IF ( std::is_function<T>::value )
        desc += "function ";
    else if CONSTEXPR_IF ( std::is_arithmetic<T>::value )
        desc += "built-in ";
    return desc + detail::get_name<T>();
}

} // namespace melanolib
#endif // MELANOLIB_UTILS_DESCRIBE_TYPE_HPP
