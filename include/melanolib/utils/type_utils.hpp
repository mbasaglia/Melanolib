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
#ifndef MELANOLIB_TYPE_UTILS_HPP
#define MELANOLIB_TYPE_UTILS_HPP

#include <iosfwd>
#include <tuple>
#include <type_traits>

namespace melanolib {


template<class... Args>
    struct DummyTuple;

template<class Head, class... Args>
    struct DummyTuple<Head, Args...> : DummyTuple<Args...>
    {
        using head = Head;
        using tail = DummyTuple<Args...>;
    };

template<>
    struct DummyTuple<>
    {
    };

/**
 * \brief Template to retrieve information about a function signature
 *
 * Use as FunctionSignature<Ret (Args...)> or FunctionSignature<Pointer>
 */
template<class T>
    struct FunctionSignature;

template<class Ret, class...Args>
    struct FunctionSignature<Ret(Args...)>
    {
        using pointer_type = Ret (*) (Args...);
        using return_type = Ret;
        using arguments_types = std::tuple<Args...>;
        using argument_types_tag = DummyTuple<Args...>;
        using invoke_types_tag = DummyTuple<Args...>;
        static constexpr auto argument_count = sizeof...(Args);
    };

template<class Ret, class...Args>
    struct FunctionSignature<Ret(*)(Args...)>
        : public FunctionSignature<Ret(Args...)>
    {
    };


/**
 * \brief Template to retrieve information about a function signature
 *
 * Use as MemberFunctionSignature<Pointer>
 */
template<class T>
    struct MemberFunctionSignature;


template<class Class, class Ret, class...Args>
    struct MemberFunctionSignature<Ret(Class::*)(Args...) const>

    {
        using pointer_type = Ret (Class::*) (Args...) const;
        using return_type = Ret;
        using arguments_types = std::tuple<Args...>;
        using argument_types_tag = DummyTuple<Args...>;
        using invoke_types_tag = DummyTuple<const Class*, Args...>;
        static constexpr auto argument_count = sizeof...(Args);
        static constexpr bool is_const = true;
    };

template<class Class, class Ret, class...Args>
    struct MemberFunctionSignature<Ret(Class::*)(Args...)>
    {
        using pointer_type = Ret (Class::*) (Args...) const;
        using return_type = Ret;
        using arguments_types = std::tuple<Args...>;
        using argument_types_tag = DummyTuple<Args...>;
        using invoke_types_tag = DummyTuple<Class*, Args...>;
        static constexpr auto argument_count = sizeof...(Args);
        static constexpr bool is_const = false;
    };

namespace detail {

    template<class Class>
        MemberFunctionSignature<decltype(&Class::operator())>
        object_to_function(Class*);

    template<class Class>
    struct PointerSignature
    {
        using argument_types_tag = DummyTuple<Class*>;
    };

    template<class Class, class Type>
        PointerSignature<Class>
        object_to_function(Type Class::*);

    struct ValueSignature
    {
        using argument_types_tag = DummyTuple<>;
    };

    ValueSignature object_to_function(void*);

    template<class Class>
        using ObjectToFunction = decltype(object_to_function(std::declval<Class*>()));

} // namespace detail

template<class Class>
    struct FunctionSignature
        : public detail::ObjectToFunction<Class>
    {
        using invoke_types_tag = typename FunctionSignature::argument_types_tag;
    };

template<class Class, class Ret, class...Args>
    struct FunctionSignature<Ret(Class::*)(Args...)>
        : public MemberFunctionSignature<Ret(Class::*)(Args...)>
    {
    };

template<class Class, class Ret, class...Args>
    struct FunctionSignature<Ret(Class::*)(Args...) const>
        : public MemberFunctionSignature<Ret(Class::*)(Args...) const>
    {
    };

/**
 * \brief Clean syntax to get a function pointer type
 */
template<class T>
    using FunctionPointer = typename FunctionSignature<T>::pointer_type;

namespace detail {
    /**
     * \brief Type used to detect the operators defined in this namespace
     */
    struct WrongOverload{};

    /**
     * \brief Type to which anything can be conveted
     */
    struct Gobble{ template<class T> Gobble(T&&); };

    /**
     * \brief Adds overload that if matched, means there is no better alternative
     */
    WrongOverload operator<< (std::ostream&, Gobble);

    /**
     * \brief Type that inherits from \c std::true_type or \c std::false_type
     *        based on whether \p T has a proper stream operator<<.
     * \note Defined in this namespace to have visibility to the \c Gobble overload.
     */
    template<class T>
    class StreamInsertable : public
        std::integral_constant<bool,
            !std::is_same<
                decltype(std::declval<std::ostream&>() << std::declval<T>()),
                WrongOverload
            >::value
        >
    {};

    /**
     * \brief Helper for IsCallable and IsCallableAnyReturn
     */
    template<class T, class Ret, class... Args>
    struct IsCallableHelper
    {
        /**
         * \brief "good" sfinae overload, returns the real return type of T()(Args...)
         */
        template<class C>
        static decltype(std::declval<C>()(std::declval<Args>()...)) test(int);

        /**
         * \brief Fall back overload
         */
        template<class C>
        static WrongOverload test(Gobble);

        /**
         * \brief Type returned by test()
         */
        using return_type = decltype(test<T>(1));

        /**
         * \brief \b true if the return type is convertible to the target
         */
        static constexpr bool convertible = std::is_convertible<return_type, Ret>::value;
        /**
         * \brief \b true if any call operator is available
         */
        static constexpr bool any = !std::is_same<return_type, WrongOverload>::value;


        /// \note could also add exact = std::is_same<return_type, Ret>::value
        ///       to check for an exact match

    };

    /**
     * \brief Type that inherits from \c std::true_type or \c std::false_type
     *        based on whether \p T has a proper operator(Args...).
     * \note C++17 introduces std::is_callable which could be used instead.
     */
    template<class T, class Ret=void, class... Args>
    class IsCallable : public
        std::integral_constant<bool, IsCallableHelper<T, Ret, Args...>::convertible>
    {};

    /**
     * \brief Type that inherits from \c std::true_type or \c std::false_type
     *        based on whether \p T has a proper operator(Args...).
     * \note C++17 introduces std::is_callable which could be used instead.
     */
    template<class T, class... Args>
    class IsCallableAnyReturn : public
        std::integral_constant<bool, IsCallableHelper<T, void, Args...>::any>
    {};
}

using detail::StreamInsertable;
using detail::IsCallable;
using detail::IsCallableAnyReturn;

/**
 * \brief Type that inherits from \c std::true_type or \c std::false_type
 *        based on whether std::decay_t<T> would do something
 */
template<class T>
    struct CanDecay : std::integral_constant<bool,
            std::is_reference<T>::value ||
            std::is_const<T>::value     ||
            std::is_volatile<T>::value  ||
            std::is_array<T>::value     >
    {};

/**
 * \brief Type that inherits from \c std::true_type or \c std::false_type
 *        based on whether \p T can be converted to \c std::string.
 */
template<class T>
    struct StringConvertible : std::is_convertible<T, std::string>
    {};


/**
 * \brief Type that inherits from \c std::true_type or \c std::false_type
 *        based on whether \p Source can be converted to \c Target
 *        with an explicit (or implicit) conversion.
 */
template <class Source, class Target>
struct ExplicitlyConvertible : public std::is_constructible<Target, Source>
{
};

/**
 * \brief SFINAE-friendly type checker
 */
template<class Type, class...Args>
    using DummyType = Type;


} // namespace melanolib
#endif // MELANOLIB_TYPE_UTILS_HPP
