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
 * GNU General Public License for more wrappers.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MELANOLIB_SCRIPTING_OBJECT_HPP
#define MELANOLIB_SCRIPTING_OBJECT_HPP

#include <vector>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <sstream>

#include "melanolib/utils/type_utils.hpp"
#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {
namespace scripting {

class Namespace;
class Object;

namespace wrapper {
    class TypeWrapper;

    /**
     * \brief Base class to erase the type of an object
     */
    class ValueWrapper
    {
    public:
        explicit ValueWrapper(const TypeWrapper* type)
            : _type(type)
        {}

        virtual ~ValueWrapper(){}

        /**
         * \brief Returns a reference to the wrapper of the type of the object
         */
        const TypeWrapper& type() const
        {
            return *_type;
        }

        /**
         * \brief Converts the contained value into a string
         * \see Stringizer
         */
        virtual std::string to_string() const = 0;

        /**
         * \brief Returns a child object (or throws MemberNotFound)
         * \throws MemberNotFound
         */
        virtual Object get_child(const std::string& name) const = 0;

        /**
         * \brief Sets a value on a child object
         * \throws MemberNotFound or TypeError
         */
        virtual void set_child(const std::string& name, const Object& value) = 0;

        /**
         * \brief Calls a child function
         * \throws MemberNotFound, FunctionError or TypeError
         */
        virtual Object call_method(const std::string& name, const std::vector<Object>& args) = 0;

    private:
        const TypeWrapper* _type;
    };
}

/**
 * \brief Exception thrown when trying to access a member that has not been exposed
 */
class MemberNotFound : std::runtime_error
{
public:
    explicit MemberNotFound(const std::string& message)
        : runtime_error(message)
    {}
};


/**
 * \brief Exception thrown when passing bad parameters to a function
 */
class FunctionError : std::invalid_argument
{
public:
    explicit FunctionError(const std::string& message)
        : invalid_argument(message)
    {}
};

/**
 * \brief Exception thrown when trying to access a type that has not been exposed
 */
class TypeError : std::logic_error
{
public:
    explicit TypeError(const std::string& message)
        : logic_error(message)
    {}
};

/**
 * \brief Main interface to access objects
 */
class Object
{
public:
    using MemberPath = std::vector<std::string>;
    using Arguments = std::vector<Object>;

    explicit Object(std::shared_ptr<wrapper::ValueWrapper> value)
        : value(std::move(value))
    {};

    /**
     * \brief Returns a wrapper of the type of the contained value
     */
    const wrapper::TypeWrapper& type() const
    {
        return value->type();
    }

    /**
     * \brief Returns an attribute
     * \throw MemberNotFound
     */
    Object get(const MemberPath& path) const
    {
        return get(path.begin(), path.end());
    }

    /**
     * \brief Returns an attribute
     * \throw MemberNotFound
     */
    Object get(const std::initializer_list<std::string>& path) const
    {
        return get(path.begin(), path.end());
    }

    /**
     * \brief Returns a direct attribute
     * \throw MemberNotFound
     */
    Object get(const std::string& name) const
    {
        return value->get_child(name);
    }

    /**
     * \brief Sets a direct attribute
     * \throw MemberNotFound or TypeError
     */
    void set(const std::string& name, const Object& new_value) const
    {
        value->set_child(name, new_value);
    }

    /**
     * \brief Invokes a member function
     * \throws MemberNotFound or TypeError
     */
    Object call(const std::string& method, const Arguments& args) const
    {
        return value->call_method(method, args);
    }

    /**
     * \brief Returns a string representing the contained object
     */
    std::string to_string() const
    {
        return value->to_string();
    }

    template<class T>
    const T& cast() const;

    template<class T>
    bool has_type() const;

private:
    template<class Iter>
        Object get(Iter begin, Iter end) const
        {
            if ( begin >= end )
                return *this;
            return value->get_child(*begin).get(begin+1, end);
        }

    std::shared_ptr<wrapper::ValueWrapper> value;
};

/**
 * \brief Object used to store values, by default stores a copy.
 *
 * Specialize if a different behaviour is required
 */
template<class T>
    struct ValueHolder
{
    ValueHolder(const T& value)
        : value(value)
    {}

    const T& get() const
    {
        return value;
    }

    T& get()
    {
        return value;
    }

    T value;
};

namespace wrapper {

    /**
     * \brief Wrapper around a type (erasure base)
     */
    class TypeWrapper
    {
    public:
        TypeWrapper(std::string&& name, Namespace* parent_namespace)
            : _name(std::move(name)),
            _parent_namespace(parent_namespace)
        {}

        virtual ~TypeWrapper(){}

        /**
         * \brief Type name (as specified when the type has been registered)
         */
        const std::string& name() const
        {
            return _name;
        }

        /**
         * \brief Typeinfo object for the wrapped type
         */
        virtual const std::type_info& type_info() const noexcept = 0;

        const Namespace& parent_namespace() const
        {
            return *_parent_namespace;
        }

    private:
        std::string _name;
        Namespace* _parent_namespace;
    };


    namespace detail {
        template<class HeldType>
            using Getter = std::function<Object(const HeldType&)>;

        template<class HeldType>
            using GetterMap = std::unordered_map<std::string, Getter<HeldType>>;

        template<class HeldType>
            using UnregGetter = std::function<Object(const HeldType&, const std::string& name)>;

        template<class HeldType>
            using Method = std::function<Object(HeldType&, const Object::Arguments&)>;

        template<class HeldType>
            using MethodMap = std::unordered_map<std::string, Method<HeldType>>;

        template<class HeldType>
            using Setter = std::function<void(HeldType&, const Object&)>;

        template<class HeldType>
            using SetterMap = std::unordered_map<std::string, Setter<HeldType>>;

        template<class HeldType>
            using UnregSetter = std::function<void(HeldType&, const std::string& name, const Object&)>;

    } // namespace detail

    /**
     * \brief Wrapper around a class
     */
    template<class Class>
        class ClassWrapper : public TypeWrapper
    {
    public:
        using HeldType = Class;

        ClassWrapper(std::string name, Namespace* parent_namespace)
            : TypeWrapper(std::move(name), parent_namespace)
        {}

        /**
         * \brief Exposes an attribute
         * \tparam T can be:
         * * A pointer to a data member of HeldType
         * * A pointer to a member function of HeldType taking no arguments
         * * Any function object taking no arguments
         * * Any function object taking a const HeldType& or a const HeldType* argument
         * * Any other object of a type registered to the parent namespace
         */
        template<class T>
            ClassWrapper& add_readonly(const std::string& name, const T& value);


        /**
         * \brief Sets a fallback functions used to get additional unregistered
         *        attributes
         * \tparam T can be:
         * * A pointer to a function member of HeldType taking a const std::string&
         * * Any function object taking a const HeldType& and a const std::string&
         * * Any function object taking a const HeldType* and a const std::string&
         * * Any function object taking a const std::string&
         */
        template<class T>
            ClassWrapper& fallback_getter(const T& functor);

        /**
         * \brief Exposes an attribute
         * \tparam Read can be:
         * * A pointer to a data member of HeldType
         * * A pointer to a member function of HeldType taking no arguments
         * * Any function object taking no arguments
         * * Any function object taking a const HeldType& or a const HeldType* argument
         * * Any other object of a type registered to the parent namespace
         * \tparam Write can be:
         * * A pointer to a data member of HeldType
         * * A pointer to a member function of HeldType taking a single argument
         * * Any function object taking a reference to HeldType and a single other argument
         * * Any function object taking a pointer to HeldType and a single other argument
         * * Any other function object or pointer taking a single argument
         */
        template<class Read, class Write>
            ClassWrapper& add_readwrite(const std::string& name, const Read& read, const Write& write);

        /**
         * \brief Exposes an attribute
         * \tparam T must be a pointer to a data member
         */
        template<class T>
            ClassWrapper& add_readwrite(const std::string& name, const T& value)
            {
                return add_readwrite(name, value, value);
            }

        /**
         * \brief Sets a fallback functions used to set additional unregistered attributes
         * \tparam T can be:
         * * A pointer to a function member of HeldType taking a const std::string& and another value
         * * Any function object taking a const HeldType&, a const std::string& and another value
         * * Any function object taking a const HeldType*, a const std::string& and another value
         * * Any function object taking a const std::string& and another value
         */
        template<class T>
            ClassWrapper& fallback_setter(const T& functor);

        /**
         * \brief Exposes a member function
         * \tparam T can be any function object or pointer.
         * If the first argument is convertible from a pointer or reference
         * to HeldType, the owning object will be passed.
         */
        template<class T>
            ClassWrapper& add_method(const std::string& name, const T& value);

        const std::type_info& type_info() const noexcept override
        {
            return typeid(HeldType);
        }

        /**
         * \brief Returns an attribute of the passed object
         * \throws MemberNotFound if \p name is not something registered
         * with add_readonly or add_readwrite
         */
        Object get_value(const HeldType& owner, const std::string& attrname) const
        {
            auto iter = getters.find(attrname);
            if ( iter == getters.end() )
            {
                if ( _fallback_getter )
                    return _fallback_getter(owner, attrname);
                throw MemberNotFound("\"" + attrname + "\" is not a member of " + name());
            }
            return iter->second(owner);
        }

        /**
         * \brief Sets the value of an attribute
         * \throws MemberNotFound if \p name is not something registered
         * with add_readwrite
         * \throws TypeError if \p value can't be properly converted
         */
        void set_value(HeldType& owner, const std::string& attrname, const Object& value) const
        {
            auto iter = setters.find(attrname);
            if ( iter == setters.end() )
            {
                if ( _fallback_setter )
                {
                    _fallback_setter(owner, attrname, value);
                    return;
                }
                throw MemberNotFound("\"" + attrname + "\" is not a writable member of " + name());
            }
            iter->second(owner, value);
        }

        /**
         * \brief Returns an attribute of the passed object
         * \throws MemberNotFound if \p name is not something registered
         * with one of the add_readonly() overloads
         */
        Object call_method(
            HeldType& owner,
            const std::string& method,
            const Object::Arguments& attributes) const
        {
            auto iter = methods.find(method);
            if ( iter == methods.end() )
            {
                throw MemberNotFound("\"" + method + "\" is not a member function of " + name());
            }
            return iter->second(owner, attributes);
        }

    private:
        detail::GetterMap<HeldType> getters;
        detail::UnregGetter<HeldType> _fallback_getter;
        detail::MethodMap<HeldType> methods;
        detail::SetterMap<HeldType> setters;
        detail::UnregSetter<HeldType> _fallback_setter;
    };

} // namespace wrapper

/**
 * \brief Functor used to convert values to string
 *
 * The default behaviour is to use stream operators if available or fallback
 * to the name of the class.
 * Specialize if a different behaviour is required
 */
template<class T>
struct Stringizer
{

    std::string operator()(const T& value, const wrapper::TypeWrapper& type) const
    {
        return to_string(value, type);
    }

    /**
     * \brief Conversion template for values that can be converted to a string
     *        using streams.
     */
    template<class TT>
        static std::enable_if_t<StreamInsertable<TT>::value, std::string>
        to_string(const TT& value, const wrapper::TypeWrapper& type)
    {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    }

    /**
     * \brief Conversion template for values that need explicit conversions
     */
    template<class TT>
        static std::enable_if_t<!StreamInsertable<TT>::value, std::string>
        to_string(const TT&, const wrapper::TypeWrapper& type)
    {
        return type.name();
    }

};

namespace wrapper {

    /**
     * \brief ValueWrapper associated with a ClassWrapper
     */
    template<class Class>
    class ObjectWrapper : public ValueWrapper
    {
    public:
        explicit ObjectWrapper(const Class& value, const ClassWrapper<Class>* type)
            : ValueWrapper(type), value(value)
        {}

        std::string to_string() const override
        {
            return Stringizer<Class>()(value.get(), type());
        }

        Object get_child(const std::string& name) const override
        {
            return class_wrapper().get_value(value.get(), name);
        }

        void set_child(const std::string& name, const Object& new_value) override
        {
            return class_wrapper().set_value(value.get(), name, new_value);
        }


        const ClassWrapper<Class>& class_wrapper() const
        {
            return static_cast<const ClassWrapper<Class>&>(type());
        }

        const Class& get() const
        {
            return value.get();
        }

        Object call_method(const std::string& name, const std::vector<Object>& args) override
        {
            return class_wrapper().call_method(value.get(), name, args);
        }

    private:
        ValueHolder<Class> value;
    };

} // namespace wrapper

/**
 * \brief Type registry
 */
class Namespace
{
public:
    /**
     * \brief Registers a type
     * \returns The registered class wrapper
     */
    template<class Type>
        wrapper::ClassWrapper<Type>& register_type(const std::string& name)
    {
        auto ptr = std::make_unique<wrapper::ClassWrapper<Type>>(name, this);
        auto& ref = *ptr;
        classes[ptr->type_info()] = std::move(ptr);
        return ref;
    }

    /**
     * \brief Registers a type using a default name
     * \returns The registered class wrapper
     */
    template<class Type>
        wrapper::ClassWrapper<Type>& register_type()
    {
        return register_type<Type>(typeid(Type).name());
    }

    /**
     * \brief Creates an object wrapper around the value
     * \throws TypeError if \p Class has not been registered with register_class
     */
    template<class Class>
    Object object(const Class& value) const
    {
        return object<Class, const Class&>(value);
    }

    /**
     * \brief Creates an object wrapper around the value
     * \throws TypeError if \p Class has not been registered with register_class
     */
    template<class Class, class... Args>
    Object object(Args&&... args) const
    {
        auto iter = classes.find(typeid(Class));
        if ( iter == classes.end() )
            throw TypeError("Unregister type");
        return Object(std::make_shared<wrapper::ObjectWrapper<Class>>(
            std::forward<Args>(args)...,
            static_cast<wrapper::ClassWrapper<Class>*>(iter->second.get())
        ));
    }

    Object object(const Object& value) const
    {
        return value;
    }

    template<class T>
    std::string type_name(bool throw_on_error = false) const
    {
        auto iter = classes.find(typeid(T));
        if ( iter == classes.end() )
        {
            if ( throw_on_error )
                throw TypeError("Unregistered type");
            return typeid(T).name();
        }
        return iter->second->name();
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<wrapper::TypeWrapper>> classes;
};

namespace wrapper {

    namespace detail {

        namespace getter {

            /**
             * \brief Exposes a data memeber as an attribute of this class
             */
            template<class HeldType, class Functor>
                std::enable_if_t<
                    std::is_member_pointer<Functor>::value ||
                    IsCallableAnyReturn<Functor, const HeldType*>::value,
                    Getter<HeldType>
                >
                register_read(
                    const ClassWrapper<HeldType>* object,
                    const Functor& functor)
                {
                    return [object, functor](const HeldType& value) {
                        return object->parent_namespace().object(
                            std::invoke(functor, &value)
                        );
                    };
                }

            /**
             * \brief Exposes an arbitraty functon (taking a const reference to the class)
             * as an attribute of this class
             */
            template<class HeldType, class Functor>
                std::enable_if_t<
                    IsCallableAnyReturn<Functor, const HeldType&>::value,
                    Getter<HeldType>
                >
                register_read(
                    const ClassWrapper<HeldType>* object,
                    const Functor& functor)
                {
                    return [object, functor](const HeldType& value) {
                        return object->parent_namespace().object(functor(value));
                    };
                }

            /**
             * \brief Exposes an arbitraty functon (taking no arguments)
             * as an attribute of this class
             */
            template<class HeldType, class Functor>
                std::enable_if_t<
                    IsCallableAnyReturn<Functor>::value,
                    Getter<HeldType>
                >
                register_read(const ClassWrapper<HeldType>* object,
                              const Functor& functor)
                {
                    return [object, functor](const HeldType&) {
                        return object->parent_namespace().object(functor());
                    };
                }

            /**
             * \brief Exposes a fixed value as an attribute of this class
             */
            template<class HeldType, class T>
                std::enable_if_t<
                    !IsCallableAnyReturn<T, const HeldType&>::value &&
                    !IsCallableAnyReturn<T>::value &&
                    !IsCallableAnyReturn<T, const HeldType*>::value &&
                    !std::is_member_pointer<T>::value,
                    Getter<HeldType>
                >
                register_read(const ClassWrapper<HeldType>* object,
                              const T& value)
                {
                    return [object, value](const HeldType&) {
                        return object->parent_namespace().object(value);
                    };
                }


            /**
             * \brief Exposes a member function as a fallback getter
             */
            template<class HeldType, class Functor>
                std::enable_if_t<
                    std::is_member_pointer<Functor>::value ||
                    IsCallableAnyReturn<Functor, const HeldType*, const std::string&>::value,
                    UnregGetter<HeldType>
                >
                unreg_getter(
                    const ClassWrapper<HeldType>* object,
                    const Functor& functor)
                {
                    return [object, functor](const HeldType& value, const std::string& name) {
                        return object->parent_namespace().object(std::invoke(functor, &value, name));
                    };
                }

            /**
             * \brief Exposes an arbitraty functon (taking a const reference to the
             * class and a name as a string) as a fallback getter
             */
            template<class HeldType, class Functor>
                std::enable_if_t<
                    IsCallableAnyReturn<Functor, const HeldType&, const std::string&>::value,
                    UnregGetter<HeldType>
                >
                unreg_getter(
                    const ClassWrapper<HeldType>* object,
                    const Functor& functor)
                {
                    return [object, functor]
                    (const HeldType& value, const std::string& name) {
                        return object->parent_namespace().object(functor(value, name));
                    };
                }

            /**
             * \brief Exposes an arbitraty functon (taking a name as a string)
             * as a fallback getter
             */
            template<class HeldType, class Functor>
                auto unreg_getter(
                    const ClassWrapper<HeldType>* object,
                    const Functor& functor,
                    std::enable_if_t<IsCallableAnyReturn<Functor, const std::string&>::value, bool> = true
                )
                {
                    return [object, functor]
                    (const HeldType&, const std::string& name) {
                        return object->parent_namespace().object(functor(name));
                    };
                }
        } // namespace getter

        namespace setter {
            /*
             * Member function or functor taking a pointer as first argument
             */
            template<class HeldType, class Functor, class Arg>
            auto setter_helper(Functor functor, HeldType& object,
                               const Object& arg, DummyTuple<Arg>)
            -> std::enable_if_t<
                std::is_member_function_pointer<Functor>::value ||
                IsCallableAnyReturn<Functor, HeldType*, Arg>::value
            >
            {
                return std::invoke(functor, object, arg.cast<Arg>());
            }

            /*
             * Function object/pointer taking a reference or value as first argument
             */
            template<class HeldType, class Functor, class Head, class Arg>
            auto setter_helper(Functor functor, HeldType& object,
                             const Object& arg, DummyTuple<Head, Arg>)
            -> std::enable_if_t<
                IsCallableAnyReturn<Functor, HeldType&, Arg>::value
            >
            {
                return std::invoke(functor, object, arg.cast<Arg>());
            }

            /**
             * Function object/pointer taking at only 1 argument
             */
            template<class HeldType, class Functor, class Arg>
            auto setter_helper(Functor functor, HeldType& object,
                const Object& arg, DummyTuple<Arg>)
            -> std::enable_if_t<
                !std::is_member_function_pointer<Functor>::value
            >
            {
                return std::invoke(functor, arg.cast<Arg>());
            }

            template<class HeldType, class Functor>
                auto register_write(
                    const ClassWrapper<HeldType>* type,
                    const Functor& functor)
                -> std::enable_if_t<
                    !std::is_member_object_pointer<Functor>::value,
                    Setter<HeldType>
                >
                {
                    return [type, functor](HeldType& value, const Object& arg) {
                        using Sig = FunctionSignature<Functor>;
                        setter_helper<HeldType>(
                                functor, value, arg,
                                typename Sig::argument_types_tag()
                        );
                    };
                }

            template<class HeldType, class Type>
                auto register_write(
                    const ClassWrapper<HeldType>* type,
                    Type HeldType::*pointer)
                -> std::enable_if_t<
                    !std::is_function<Type>::value,
                    Setter<HeldType>
                >
                {
                    return [type, pointer](HeldType& value, const Object& arg) {
                        value.*pointer = arg.cast<Type>();
                    };
                }

            /*
             * Member function or functor taking a pointer as first argument
             */
            template<class HeldType, class Functor, class String, class Arg>
            auto unreg_setter_helper(
                Functor functor, HeldType& object, const std::string& name,
                const Object& arg, DummyTuple<String, Arg>)
            -> std::enable_if_t<
                std::is_member_function_pointer<Functor>::value ||
                IsCallableAnyReturn<Functor, HeldType*, std::string, Arg>::value
            >
            {
                return std::invoke(functor, object, name, arg.cast<Arg>());
            }

            /*
             * Function object/pointer taking a reference or value as first argument
             */
            template<class HeldType, class Functor, class Head, class String, class Arg>
            auto unreg_setter_helper(
                Functor functor, HeldType& object, const std::string& name,
                const Object& arg, DummyTuple<Head, String, Arg>)
            -> std::enable_if_t<
                IsCallableAnyReturn<Functor, HeldType&, std::string, Arg>::value
            >
            {
                return std::invoke(functor, object, name, arg.cast<Arg>());
            }

            /**
             * Function object/pointer taking at only name and value
             */
            template<class HeldType, class Functor, class String, class Arg>
            auto unreg_setter_helper(
                Functor functor, HeldType& object,  const std::string& name,
                const Object& arg, DummyTuple<String, Arg>)
            -> std::enable_if_t<
                !std::is_member_function_pointer<Functor>::value
            >
            {
                return std::invoke(functor, name, arg.cast<Arg>());
            }

            template<class HeldType, class Functor>
                auto unreg_setter(
                    const ClassWrapper<HeldType>* type,
                    const Functor& functor)
                -> UnregSetter<HeldType>
                {
                    return [type, functor](HeldType& object, const std::string& name, const Object& arg) {
                        using Sig = FunctionSignature<Functor>;
                        unreg_setter_helper<HeldType>(
                            functor, object, name, arg,
                            typename Sig::argument_types_tag()
                        );
                    };
                }
        } // namespace setter

        namespace function {
            /**
             * \brief Dummy class to a get compile-time sequence of indices from a parameter pack
             * \tparam Indices Sequence if indices from 0 to sizeof...(Indices)
             */
            template <int... Indices>
                struct IndexPack {};

            /**
             * \brief Dummy class that builds an integer pack from a single integer
             * \tparam N The number which needs to be converted to a pack
             * \tparam Indices Pack of indices for \c IndexPack, starts out empty
             *
             * It expands \p N recursively into an int pack, the last recursive
             * class inherits IndexPack, which only has the int pack as template
             * parameter.
             */
            template <int N, int... Indices>
                struct IndexPackBuilder : IndexPackBuilder<N-1, N-1, Indices...> {};

            /**
            * \brief Termination for \c IndexPackBuilder
            */
            template <int... Indices>
                struct IndexPackBuilder<0, Indices...> : IndexPack<Indices...> {};

            /*
             * Member function or functor taking a pointer as first argument
             */
            template<class HeldType, class Ret, class Functor, class... Args, int... Indices>
            auto call_helper(
                Functor functor,
                HeldType& object,
                const Object::Arguments& args,
                DummyTuple<Args...>,
                IndexPack<Indices...>)
            -> std::enable_if_t<
                std::is_member_function_pointer<Functor>::value ||
                IsCallableAnyReturn<Functor, HeldType*, Args...>::value,
                Ret
            >
            {
                if ( args.size() != sizeof...(Args) )
                    throw FunctionError("Wrong number of arguments");
                return std::invoke(functor, object, args[Indices].cast<Args>()...);
            }

            /*
             * Function object/pointer taking a reference or value as first argument
             */
            template<class HeldType, class Ret, class Functor, class Head, class... Args, int... Indices>
            auto call_helper(
                Functor functor,
                HeldType& object,
                const Object::Arguments& args,
                DummyTuple<Head, Args...>,
                IndexPack<0, Indices...>)
            -> std::enable_if_t<
                IsCallableAnyReturn<Functor, HeldType&, Args...>::value,
                Ret
            >
            {
                if ( args.size() != sizeof...(Args) )
                    throw FunctionError("Wrong number of arguments");
                return std::invoke(functor, object, args[Indices-1].cast<Args>()...);
            }

            /**
             * Function object/pointer taking at least 1 argument
             */
            template<class HeldType, class Ret, class Functor, class Head, class... Args, int... Indices>
            auto call_helper(
                Functor functor,
                HeldType& object,
                const Object::Arguments& args,
                DummyTuple<Head, Args...>,
                IndexPack<0, Indices...>)
            -> std::enable_if_t<
                !std::is_member_function_pointer<Functor>::value &&
                !std::is_convertible<HeldType&, Head>::value &&
                !std::is_convertible<HeldType*, Head>::value,
                Ret
            >
            {
                if ( args.size() != sizeof...(Args) + 1 )
                    throw FunctionError("Wrong number of arguments");
                return std::invoke(functor, args[0].cast<Head>(), args[Indices].cast<Args>()...);
            }

            /**
             * Function object/pointer taking at least 1 argument
             */
            template<class HeldType, class Ret, class Functor>
            auto call_helper(
                Functor functor,
                HeldType& object,
                const Object::Arguments& args,
                DummyTuple<>,
                IndexPack<>)
            -> std::enable_if_t<
                !std::is_member_function_pointer<Functor>::value,
                Ret
            >
            {
                if ( args.size() != 0 )
                    throw FunctionError("Wrong number of arguments");
                return std::invoke(functor);
            }

            /** Member function
             * \brief Exposes a memeber function as a method of the class
             */
            template<class HeldType, class Functor>
            auto wrap_functor(
                const ClassWrapper<HeldType>* type,
                const std::string& name,
                Functor functor)
            {
                return [type, functor](HeldType& value, const Object::Arguments& args) {
                    using Sig = FunctionSignature<Functor>;
                    return type->parent_namespace().object(
                        call_helper<HeldType, typename Sig::return_type>(
                            functor, value, args,
                            typename Sig::argument_types_tag(),
                            IndexPackBuilder<Sig::argument_count>()
                        )
                    );
                };
            }

        } // namespace function

    } // namespace detail

    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::add_readonly(const std::string& name, const T& value)
        {
            getters[name] = detail::getter::register_read<HeldType>(this, value);
            return *this;
        }


    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::fallback_getter(const T& functor)
        {
            _fallback_getter = detail::getter::unreg_getter<HeldType>(this, functor);
            return *this;
        }

    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::add_method(const std::string& name, const T& value)
        {
            methods[name] = detail::function::wrap_functor<HeldType>(this, name, value);
            return *this;
        }

    template<class Class>
    template<class Read, class Write>
        ClassWrapper<Class>& ClassWrapper<Class>::add_readwrite(
            const std::string& name, const Read& read, const Write& write)
        {
            getters[name] = detail::getter::register_read<HeldType>(this, read);
            setters[name] = detail::setter::register_write<HeldType>(this, write);
            return *this;
        }

    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::fallback_setter(const T& functor)
        {
            _fallback_setter = detail::setter::unreg_setter<HeldType>(this, functor);
            return *this;
        }

} // namespace wrapper

template<class T>
const T& Object::cast() const
{
    /// \todo Allow binding references
    using Type = std::decay_t<T>;
    if ( auto ptr = dynamic_cast<wrapper::ObjectWrapper<Type>*>(value.get()) )
        return ptr->get();
    throw TypeError(
        "Object is of type " + value->type().name() + ", not "
        + value->type().parent_namespace().type_name<Type>()
    );
}

template<class T>
bool Object::has_type() const
{
    /// \todo Allow binding references
    using Type = std::decay_t<T>;
    return dynamic_cast<wrapper::ObjectWrapper<Type>*>(value.get());
}


} // namespace scripting
} // namespace melanolib
#endif // MELANOLIB_SCRIPTING_OBJECT_HPP
