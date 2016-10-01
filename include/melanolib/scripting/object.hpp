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
#include <unordered_map>
#include <memory>
#include <sstream>

#include "melanolib/utils/type_utils.hpp"

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

    private:
        const TypeWrapper* _type;
    };
}

/**
 * \brief Main interface to access objects
 */
class Object
{
public:
    using MemberPath = std::vector<std::string>;
    using MemberPathIterator = MemberPath::const_iterator;

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
    Object get(const MemberPath path) const
    {
        return get(path.begin(), path.end());
    }

    /**
     * \brief Returns a string representing the contained object
     */
    std::string to_string() const
    {
        return value->to_string();
    }

private:
    Object get(MemberPathIterator begin, MemberPathIterator end) const
    {
        if ( begin >= end )
            return *this;
        return value->get_child(*begin).get(begin+1, end);
    }

    std::shared_ptr<wrapper::ValueWrapper> value;
};

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
 * \brief Exception thrown when trying to access a type that has not been exposed
 */
class ClassNotFound : std::logic_error
{
public:
    explicit ClassNotFound(const std::string& message)
        : logic_error(message)
    {}
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

    T value;
};

namespace wrapper {

    /**
     * \brief Wrapper around a type (erasure base)
     */
    class TypeWrapper
    {
    public:
        TypeWrapper(std::string&& name)
            : _name(std::move(name))
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

    private:
        std::string _name;
    };


    namespace detail {
        template<class HeldType>
            using Getter = std::function<Object(const HeldType&)>;

        template<class HeldType>
            using GetterMap = std::unordered_map<std::string, Getter<HeldType>>;

        template<class HeldType>
            using UnregGetter = std::function<Object(const HeldType&, const std::string& name)>;

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
            : TypeWrapper(std::move(name)),
            _parent_namespace(parent_namespace)
        {}

        /**
         * \brief Exposes an attribute
         * \tparam T can be:
         * * A pointer to a data/function member of HeldType
         * * Any function object taking no arguments
         * * Any function object taking a const HeldType& argument
         * * Any other type registered to the parent namespace
         */
        template<class T>
            ClassWrapper& add_readonly(const std::string& name, const T& value);

        /**
         * \brief Sets a fallback functions used to get additional unregistered
         *        attributes
         * \tparam T can be:
         * * A pointer to a function member of HeldType taking a const std::string&
         * * Any function object taking a const HeldType& and a const std::string&
         * * Any function object taking a const std::string&
         */
        template<class T>
            ClassWrapper& fallback_getter(const T& functor);

        const std::type_info& type_info() const noexcept override
        {
            return typeid(HeldType);
        }

        /**
         * \brief Returns an attribute of the passed object
         * \throws MemberNotFound if \p name is not something registered
         * with one of the add_readonly() overloads
         */
        Object get_child(const HeldType& owner, const std::string& attrname) const
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

        const Namespace& parent_namespace() const
        {
            return *_parent_namespace;
        }

    private:
        detail::GetterMap<HeldType> getters;
        detail::UnregGetter<HeldType> _fallback_getter;
        Namespace* _parent_namespace;
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
            return class_wrapper().get_child(value.get(), name);
        }

        const ClassWrapper<Class>& class_wrapper() const
        {
            return static_cast<const ClassWrapper<Class>&>(type());
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
     * \throws ClassNotFound if \p Class has not been registered with register_class
     */
    template<class Class>
    Object object(const Class& value) const
    {
        auto iter = classes.find(typeid(Class));
        if ( iter == classes.end() )
            throw ClassNotFound("Unregister type");
        return Object(std::make_shared<wrapper::ObjectWrapper<Class>>(
            value,
            static_cast<wrapper::ClassWrapper<Class>*>(iter->second.get())
        ));
    }

    Object object(const Object& value) const
    {
        return value;
    }

private:
    using TypeInfo = std::reference_wrapper<const std::type_info>;

    struct TypeHasher
    {
        std::size_t operator()(TypeInfo info) const
        {
            return info.get().hash_code();
        }
    };

    struct TypeEqualTo
    {
        bool operator()(TypeInfo lhs, TypeInfo rhs) const
        {
            return lhs.get() == rhs.get();
        }
    };

    std::unordered_map<TypeInfo, std::unique_ptr<wrapper::TypeWrapper>,
                       TypeHasher, TypeEqualTo> classes;
};

namespace wrapper {

    namespace detail {
        template<class HeldType>
            using Getter = std::function<Object(const HeldType&)>;

        template<class HeldType>
            using GetterMap = std::unordered_map<std::string, Getter<HeldType>>;

        /**
         * \brief Exposes a data memeber as an attribute of this class
         */
        template<class HeldType, class T>
            void register_read(GetterMap<HeldType>& getters,
                               const ClassWrapper<HeldType>* object,
                               const std::string& name,
                               T HeldType::* pointer)
            {
                getters[name] = [object, pointer](const HeldType& value) {
                    return object->parent_namespace().object(value.*pointer);
                };
            }

        /**
         * \brief Exposes a memeber function as an attribute of this class
         */
        template<class HeldType, class T>
            void register_read(GetterMap<HeldType>& getters,
                               const ClassWrapper<HeldType>* object,
                               const std::string& name,
                               T (HeldType::*pointer)() const)
            {
                getters[name] = [object, pointer](const HeldType& value) {
                    return object->parent_namespace().object((value.*pointer)());
                };
            }

        /**
         * \brief Exposes an arbitraty functon (taking a const reference to the class)
         * as an attribute of this class
         */
        template<class HeldType, class Functor>
            std::enable_if_t<IsCallableAnyReturn<Functor, const HeldType&>::value>
            register_read(GetterMap<HeldType>& getters,
                          const ClassWrapper<HeldType>* object,
                          const std::string& name,
                          const Functor& functor)
            {
                getters[name] = [object, functor](const HeldType& value) {
                    return object->parent_namespace().object(functor(value));
                };
            }

        /**
         * \brief Exposes an arbitraty functon (taking no arguments)
         * as an attribute of this class
         */
        template<class HeldType, class Functor>
            std::enable_if_t<IsCallableAnyReturn<Functor>::value>
            register_read(GetterMap<HeldType>& getters,
                          const ClassWrapper<HeldType>* object,
                          const std::string& name,
                          const Functor& functor)
            {
                getters[name] = [object, functor](const HeldType&) {
                    return object->parent_namespace().object(functor());
                };
            }

        /**
         * \brief Exposes a fixed value as an attribute of this class
         */
        template<class HeldType, class T>
            std::enable_if_t<
                !IsCallableAnyReturn<T, const HeldType&>::value &&
                !IsCallableAnyReturn<T>::value>
            register_read(GetterMap<HeldType>& getters,
                          const ClassWrapper<HeldType>* object,
                          const std::string& name,
                          const T& value)
            {
                getters[name] = [object, value](const HeldType&) {
                    return object->parent_namespace().object(value);
                };
            }


        /**
         * \brief Exposes a member function as a fallback getter
         */
        template<class HeldType, class T>
            auto unreg_getter(
                const ClassWrapper<HeldType>* object,
                T (HeldType::*pointer)(const std::string&) const)
            {
                return [object, pointer](const HeldType& value, const std::string& name) {
                    return object->parent_namespace().object((value.*pointer)(name));
                };
            }

        /**
         * \brief Exposes an arbitraty functon (taking a const reference to the
         * class and a name as a string) as a fallback getter
         */
        template<class HeldType, class Functor>
            auto unreg_getter(
                const ClassWrapper<HeldType>* object,
                const Functor& functor,
                std::enable_if_t<IsCallableAnyReturn<Functor, const HeldType&, const std::string&>::value, bool> = true
            )
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

    } // namespace detail

    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::add_readonly(const std::string& name, const T& value)
        {
            detail::register_read<HeldType>(getters, this, name, value);
            return *this;
        }


    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::fallback_getter(const T& functor)
        {
            _fallback_getter = detail::unreg_getter<HeldType>(this, functor);
            return *this;
        }
} // namespace wrapper

} // namespace scripting
} // namespace melanolib
#endif // MELANOLIB_SCRIPTING_OBJECT_HPP
