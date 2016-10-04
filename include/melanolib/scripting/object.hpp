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
#include <utility>

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
         * \throws MemberNotFound
         */
        virtual Object call_method(const std::string& name, const std::vector<Object>& args) = 0;

        /**
         * \brief Converts to an object of a different type
         */
        virtual Object converted(const std::type_info& type) const = 0;

    private:
        const TypeWrapper* _type;
    };
}

/**
 * \brief Exception thrown when trying to access a member that has not been exposed
 */
class MemberNotFound : public std::runtime_error
{
public:
    explicit MemberNotFound(const std::string& message)
        : runtime_error(message)
    {}
};

/**
 * \brief Exception thrown when trying to access a type that has not been exposed
 */
class TypeError : public std::logic_error
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

    /**
     * \brief Cast to a reference of the contained type
     * \throws TypeError if the type doesn't match
     */
    template<class T>
    const T& cast() const;

    /**
     * \brief Whether it's safe to call cast<T>()
     */
    template<class T>
    bool has_type() const;

    /**
     * \brief Convert the internal representation using its type registered conversions
     * \throw MemberNotFound if the conversion is not available
     */
    template<class T>
    Object& convert()
    {
        *this = converted<T>();
        return *this;
    }

    /**
     * \brief Returns an object with the internal representation converted
     * using this object's type registered conversions
     * \throw MemberNotFound if the conversion is not available
     */
    template<class T>
    Object converted() const;

    /**
     * \brief Performs conversion and the cast()
     * \note This returns a copy of the returned value,
     * use convert() or converted() followed by a cast() if you require a reference
     */
    template<class T>
    T converted_cast() const
    {
        return converted<T>().cast<T>();
    }

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

template<class T>
using Ref = std::reference_wrapper<T>;

template<class T>
Ref<T> wrap_reference(T& reference)
{
    return reference;
}

struct CopyPolicy{};
struct WrapReferencePolicy{};

/**
 * \brief Object used to store values, by default stores a copy.
 *
 * Specialize if a different behaviour is required
 */
template<class T>
    struct ValueHolder
{
    ValueHolder(const T& value)
        : holder(value)
    {}

    ValueHolder(T* pointer)
        : holder(pointer)
    {}

    ValueHolder(const Ref<T>& reference)
        : holder(&reference.get())
    {}

    const T& get() const
    {
        if ( holder.which() == 0 )
            return melanolib::get<T>(holder);
        return *melanolib::get<T*>(holder);
    }

    T& get()
    {
        if ( holder.which() == 0 )
            return melanolib::get<T>(holder);
        return *melanolib::get<T*>(holder);;
    }

    melanolib::Variant<T, T*> holder;
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
        virtual std::type_index type_index() const noexcept = 0;

        const Namespace& parent_namespace() const
        {
            return *_parent_namespace;
        }

        /**
         * \brief Calls a dynamic constructor
         */
        virtual Object make_object(const Object::Arguments& arguments) const = 0;

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

        template<class Return, class... FixedArgs>
        class Overloadable
        {
        public:
            using Functor = std::function<Return(FixedArgs..., const Object::Arguments&)>;
            using TypeList = std::vector<std::type_index>;

            template<class FunctorT, class... Args>
            Overloadable(DummyTuple<Args...>, FunctorT&& functor)
                : functor(std::forward<FunctorT>(functor)),
                  types({std::type_index(typeid(Args))...})
            {}

            bool can_call(const Object::Arguments& args) const
            {
                if ( args.size() != types.size() )
                    return false;
                auto type_iter = types.begin();
                for ( const Object& arg : args )
                {
                    if ( arg.type().type_index() != *type_iter )
                        return false;
                    ++type_iter;
                }
                return true;
            }

            /**
             * \pre can_call(args)
             */
            Return operator()(FixedArgs... pre_args, const Object::Arguments& args) const
            {
                if ( args.size() != types.size() )
                    throw TypeError("Wrong number of arguments");
                return functor(pre_args..., args);
            }
        private:
            Functor functor;
            TypeList types;
        };


        template<class HeldType>
            using Method = Overloadable<Object, HeldType&>;

        template<class HeldType>
            using MethodMap = std::unordered_multimap<std::string, Method<HeldType>>;

        template<class HeldType>
            using Setter = std::function<void(HeldType&, const Object&)>;

        template<class HeldType>
            using SetterMap = std::unordered_map<std::string, Setter<HeldType>>;

        template<class HeldType>
            using UnregSetter = std::function<void(HeldType&, const std::string& name, const Object&)>;

        template<class HeldType>
            using Constructor = Overloadable<Object>;

        template<class HeldType>
            using ConstrorList = std::vector<Constructor<HeldType>>;

        template<class HeldType>
            using ConverterMap = std::unordered_map<std::type_index, Getter<HeldType>>;

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
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class T, class ReturnPolicy = CopyPolicy>
            ClassWrapper& add_readonly(const std::string& name, const T& value, ReturnPolicy = {});


        /**
         * \brief Sets a fallback functions used to get additional unregistered
         *        attributes
         * \tparam T can be:
         * * A pointer to a function member of HeldType taking a const std::string&
         * * Any function object taking a const HeldType& and a const std::string&
         * * Any function object taking a const HeldType* and a const std::string&
         * * Any function object taking a const std::string&
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class T, class ReturnPolicy = CopyPolicy>
            ClassWrapper& fallback_getter(const T& functor, ReturnPolicy = {});

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
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class Read, class Write, class ReturnPolicy = CopyPolicy>
            ClassWrapper& add_readwrite(
                const std::string& name,
                const Read& read,
                const Write& write,
                ReturnPolicy = {});

        /**
         * \brief Exposes an attribute
         * \tparam T must be a pointer to a data member
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class T>
            ClassWrapper& add_readwrite(const std::string& name, const T& value,
                                        CopyPolicy = {})
            {
                return add_readwrite(name, value, value, CopyPolicy{});
            }

        template<class T>
            ClassWrapper& add_readwrite(const std::string& name, const T& value,
                                        WrapReferencePolicy)
            {
                return add_readwrite(name, value, value, WrapReferencePolicy{});
            }

        /**
         * \brief Sets a fallback function used to set additional unregistered attributes
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
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class T, class ReturnPolicy = CopyPolicy>
            ClassWrapper& add_method(const std::string& name, const T& value,
                                     ReturnPolicy = {});

        /**
         * \brief Sets the function used as a constructor
         * \tparam T can be any callable object
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class T, class ReturnPolicy = CopyPolicy>
            ClassWrapper& constructor(const T& functor, ReturnPolicy = {});

        /**
         * \brief Exposes a constructor
         * \tparam Args constructor arguments
         */
        template<class... Args>
            ClassWrapper& constructor();

        /**
         * \brief Exposes a conversion operator
         * \tparam Functor can be:
         * * A pointer to a data member of HeldType
         * * A pointer to a member function of HeldType taking no arguments
         * * Any function object taking no arguments
         * * Any function object taking a const HeldType& or a const HeldType* argument
         * * Any other object of a type registered to the parent namespace
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class Target, class Functor, class ReturnPolicy = CopyPolicy>
            ClassWrapper& conversion(const Functor& functor, ReturnPolicy = {});

        /**
         * \brief Exposes a conversion operator
         * \tparam Functor can be:
         * * A pointer to a data member of HeldType
         * * A pointer to a member function of HeldType taking no arguments
         * * Any function object taking no arguments
         * * Any function object taking a const HeldType& or a const HeldType* argument
         * * Any other object of a type registered to the parent namespace
         */
        template<class Functor, class ReturnPolicy = CopyPolicy>
            ClassWrapper& conversion(const Functor& functor, ReturnPolicy = {});

        std::type_index type_index() const noexcept override
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
            const Object::Arguments& arguments) const
        {
            auto range = methods.equal_range(method);
            if ( range.first == range.second )
                throw MemberNotFound("\"" + method + "\" is not a member function of " + name());

            for ( auto it = range.first; it != range.second; ++it )
                if ( it->second.can_call(arguments) )
                    return it->second(owner, arguments);
            throw MemberNotFound("No matching overload of \"" + method + "\" in " + name());
        }

        /**
         * \brief Calls a dynamic constructor
         */
        Object make_object(const Object::Arguments& arguments) const override;

        /**
         * \brief Returns an Object with a converted type
         * \throws MemberNotFound if \p name is not something registered
         * with conversion()
         */
        Object convert(const HeldType& owner, const std::type_info& type) const
        {
            auto iter = converters.find(type);
            if ( iter == converters.end() )
            {
                throw MemberNotFound("Cannot convert " + name() + " to " +
                    parent_namespace().type_name(type));
            }
            return iter->second(owner);
        }

    private:
        detail::GetterMap<HeldType> getters;
        detail::UnregGetter<HeldType> _fallback_getter;
        detail::MethodMap<HeldType> methods;
        detail::SetterMap<HeldType> setters;
        detail::UnregSetter<HeldType> _fallback_setter;
        detail::ConstrorList<HeldType> _constructors;
        detail::ConverterMap<HeldType> converters;
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

        explicit ObjectWrapper(const Ref<Class>& reference, const ClassWrapper<Class>* type)
            : ValueWrapper(type), value(reference)
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

        Object converted(const std::type_info& type) const override
        {
            return class_wrapper().convert(value.get(), type);
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
        classes[ptr->type_index()] = std::move(ptr);
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
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object bind(const Class& value, CopyPolicy) const
    {
        return object(value);
    }

    /**
     * \brief Creates an object wrapper around the reference
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object bind(Class& value, WrapReferencePolicy) const
    {
        return reference(value);
    }

    /**
     * \warning Evil bind, needed to bind direct members as getters that should
     *          be wrapped as references (which can't be const by definition)
     */
    template<class Class>
    Object bind(const Class& value, WrapReferencePolicy) const
    {
        return reference(const_cast<Class&>(value));
    }

    /**
     * \brief Creates an object wrapper around the value
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object object(const Class& value) const
    {
        return build_object<Class>(value);
    }

    /**
     * \brief Creates an object wrapper around the reference
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object object(const Ref<Class>& reference) const
    {
        return build_object<Class>(reference);
    }

    /**
     * \brief Creates an object wrapper around the reference
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object reference(Class& reference) const
    {
        return build_object<Class>(wrap_reference(reference));
    }

    /**
     * \brief Creates an object wrapper around the value
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class, class... Args>
    Object object(Args&&... args) const
    {
        return build_object<Class>(Class(std::forward<Args>(args)...));
    }

    /**
     * \brief Overload to forward Object instances without additional wrapping
     */
    Object object(const Object& value) const
    {
        return value;
    }

    /**
     * \brief Overload to forward Object instances without additional wrapping
     *
     * Objects share their state between copies anyway
     */
    Object object(const Ref<Object>& reference) const
    {
        return reference.get();
    }

    /**
     * \brief Overload to forward Object instances without additional wrapping
     *
     * Objects share their state between copies anyway
     */
    Object reference(Object& reference) const
    {
        return reference;
    }

    /**
     * \brief Creates an object from run-time values
     * \throws TypeError if the type has not been registered
     * \note It's possible for multiple types to have the same name, if that
     * is the case it is not specified which one will be selected.
     */
    Object object(const std::string& class_name, const Object::Arguments& args) const
    {
        for ( const auto& p : classes )
        {
            if ( p.second->name() == class_name )
            {
                return p.second->make_object(args);
            }
        }
        throw TypeError("Unregister type: " + class_name);
    }

    template<class T>
    std::string type_name(bool throw_on_error = false) const
    {
        return type_name(typeid(T), throw_on_error);
    }

    std::string type_name(const std::type_info& type, bool throw_on_error = false) const
    {
        auto iter = classes.find(type);
        if ( iter == classes.end() )
        {
            if ( throw_on_error )
                throw TypeError("Unregistered type");
            return type.name();
        }
        return iter->second->name();
    }

private:
    template<class Class, class Ctor>
    Object build_object(Ctor&& ctor_arg) const
    {
        auto iter = classes.find(typeid(Class));
        if ( iter == classes.end() )
            throw TypeError("Unregister type");
        return Object(std::make_shared<wrapper::ObjectWrapper<Class>>(
            std::forward<Ctor>(ctor_arg),
            static_cast<wrapper::ClassWrapper<Class>*>(iter->second.get())
        ));
    }

    std::unordered_map<std::type_index, std::unique_ptr<wrapper::TypeWrapper>> classes;
};

namespace wrapper {

    namespace detail {

        namespace getter {

            template<class GetterType, class HeldType, class Functor, class ReturnPolicy>
                struct GetterBase
            {
                Object operator()(const HeldType& value) const
                {
                    return type->parent_namespace().bind(
                        GetterType::invoke(functor, value),
                        ReturnPolicy{}
                    );
                }

                using ReturnType = decltype(GetterType::invoke(
                    std::declval<Functor>(),
                    std::declval<const HeldType&>()));

                const ClassWrapper<HeldType>* type;
                Functor functor;
            };

            struct GetterPointer
            {
                template<class HeldType, class Functor>
                    static decltype(auto) invoke(const Functor& functor, const HeldType& value)
                    {
                        return std::invoke(functor, &value);
                    }
            };

            /**
             * Member pointer or function taking a const HeldType*
             */
            template<class ReturnPolicy, class HeldType, class Functor>
                std::enable_if_t<
                    std::is_member_pointer<Functor>::value ||
                    IsCallableAnyReturn<Functor, const HeldType*>::value,
                    GetterBase<GetterPointer, HeldType, Functor, ReturnPolicy>
                >
                wrap_getter(const ClassWrapper<HeldType>* object, const Functor& functor)
                {
                    return {object, functor};
                }

            struct GetterReference
            {
                template<class HeldType, class Functor>
                    static decltype(auto) invoke(const Functor& functor, const HeldType& value)
                    {
                        return std::invoke(functor, value);
                    }
            };

            /**
             * Functon taking a const reference to the class
             */
            template<class ReturnPolicy, class HeldType, class Functor>
                std::enable_if_t<
                    IsCallableAnyReturn<Functor, const HeldType&>::value,
                    GetterBase<GetterReference, HeldType, Functor, ReturnPolicy>
                >
                wrap_getter(const ClassWrapper<HeldType>* object, const Functor& functor)
                {
                    return {object, functor};
                }

            struct GetterNoarg
            {
                template<class HeldType, class Functor>
                    static decltype(auto) invoke(const Functor& functor, const HeldType& value)
                    {
                        return std::invoke(functor);
                    }
            };

            /**
             * Arbitraty functon taking no arguments
             */
            template<class ReturnPolicy, class HeldType, class Functor>
                std::enable_if_t<
                    IsCallableAnyReturn<Functor>::value,
                    GetterBase<GetterNoarg, HeldType, Functor, ReturnPolicy>
                >
                wrap_getter(const ClassWrapper<HeldType>* object, const Functor& functor)
                {
                    return {object, functor};
                }

            template<class Policy> struct GetterFixed{};

            template<>
            struct GetterFixed<CopyPolicy>
            {
                template<class HeldType, class Functor>
                    static Functor invoke(const Functor& functor, const HeldType& value)
                    {
                        return functor;
                    }

                template<class HeldType, class Value>
                using Base = GetterBase<GetterFixed, HeldType, Value, CopyPolicy>;
            };

            template<>
            struct GetterFixed<WrapReferencePolicy>
            {
                template<class HeldType, class Functor>
                    static Functor& invoke(Functor& functor, const HeldType& value)
                    {
                        return functor;
                    }

                template<class HeldType, class Value>
                using Base = GetterBase<GetterFixed, HeldType, Value&, WrapReferencePolicy>;
            };

            /**
             * Fixed value
             */
            template<class ReturnPolicy, class HeldType, class T>
                std::enable_if_t<
                    !IsCallableAnyReturn<T, const HeldType&>::value &&
                    !IsCallableAnyReturn<T>::value &&
                    !IsCallableAnyReturn<T, const HeldType*>::value &&
                    !std::is_member_pointer<T>::value,
                    typename GetterFixed<ReturnPolicy>::template Base<HeldType, T>
                >
                wrap_getter(const ClassWrapper<HeldType>* object, const T& value)
                {
                    return {object, const_cast<T&>(value)};
                }


            /**
             * \brief Exposes a member function as a fallback getter
             */
            template<class ReturnPolicy, class HeldType, class Functor>
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
                        return object->parent_namespace().bind(
                            std::invoke(functor, &value, name),
                            ReturnPolicy{}
                        );
                    };
                }

            /**
             * \brief Exposes an arbitraty functon (taking a const reference to the
             * class and a name as a string) as a fallback getter
             */
            template<class ReturnPolicy, class HeldType, class Functor>
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
                        return object->parent_namespace().bind(
                            functor(value, name),
                            ReturnPolicy{}
                        );
                    };
                }

            /**
             * \brief Exposes an arbitraty functon (taking a name as a string)
             * as a fallback getter
             */
            template<class ReturnPolicy, class HeldType, class Functor>
                auto unreg_getter(
                    const ClassWrapper<HeldType>* object,
                    const Functor& functor,
                    std::enable_if_t<IsCallableAnyReturn<Functor, const std::string&>::value, bool> = true
                )
                {
                    return [object, functor]
                    (const HeldType&, const std::string& name) {
                        return object->parent_namespace().bind(
                            functor(name),
                            ReturnPolicy{}
                        );
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
                auto wrap_setter(
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
                auto wrap_setter(
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

            template<class MethodType, class HeldType, class Functor, class ReturnPolicy, class... Args>
            struct MethodBase
            {
                auto operator()(HeldType& value, const Object::Arguments& args) const
                {
                    return type->parent_namespace().bind(
                        MethodType::template invoke<HeldType, Functor, Args...>(
                            functor, value, args,
                            std::make_index_sequence<sizeof...(Args)>{}
                        ),
                        ReturnPolicy{}
                    );
                }

                Method<HeldType> method() const
                {
                    return {DummyTuple<Args...>{}, *this};
                }

                const ClassWrapper<HeldType>* type;
                Functor functor;
            };

            /*
             * Member function or functor taking a pointer as first argument
             */
            struct MethodPointer
            {
                template<class HeldType, class Functor, class... Args, std::size_t... Indices>
                    static auto invoke(const Functor& functor,
                                       HeldType& value,
                                       const Object::Arguments& args,
                                       std::index_sequence<Indices...>)
                    {
                        return  std::invoke(functor, value, args[Indices].cast<Args>()...);
                    }
            };

            template<class ReturnPolicy, class HeldType, class Functor, class... Args>
            auto call_helper(
                const ClassWrapper<HeldType>* type,
                Functor functor,
                DummyTuple<Args...>)
            -> std::enable_if_t<
                std::is_member_function_pointer<Functor>::value ||
                IsCallableAnyReturn<Functor, HeldType*, Args...>::value,
                MethodBase<MethodPointer, HeldType, Functor, ReturnPolicy, Args...>
            > { return {type, functor}; }

            /*
             * Function object/pointer taking a reference or value as first argument
             */
            struct MethodReference
            {
                template<class HeldType, class Functor, class... Args, std::size_t... Indices>
                    static auto invoke(const Functor& functor,
                                       HeldType& value,
                                       const Object::Arguments& args,
                                       std::index_sequence<Indices...>)
                    {
                        return std::invoke(functor, value, args[Indices].cast<Args>()...);
                    }
            };
            template<class ReturnPolicy, class HeldType, class Functor, class Head, class... Args>
            auto call_helper(
                const ClassWrapper<HeldType>* type,
                Functor functor,
                DummyTuple<Head, Args...>)
            -> std::enable_if_t<
                IsCallableAnyReturn<Functor, HeldType&, Args...>::value,
                MethodBase<MethodReference, HeldType, Functor, ReturnPolicy, Args...>
            > { return {type, functor}; }

            /**
             * Function object/pointer taking at least 1 argument
             */
            struct MethodOther
            {
                template<class HeldType, class Functor, class... Args, std::size_t... Indices>
                    static auto invoke(const Functor& functor,
                                       HeldType& value,
                                       const Object::Arguments& args,
                                       std::index_sequence<Indices...>)
                    {
                        return std::invoke(functor, args[Indices].cast<Args>()...);
                    }
            };
            template<class ReturnPolicy, class HeldType, class Functor, class Head, class... Args>
            auto call_helper(
                const ClassWrapper<HeldType>* type,
                Functor functor,
                DummyTuple<Head, Args...>)
            -> std::enable_if_t<
                !std::is_member_function_pointer<Functor>::value &&
                !std::is_convertible<HeldType&, Head>::value &&
                !std::is_convertible<HeldType*, Head>::value,
                MethodBase<MethodOther, HeldType, Functor, ReturnPolicy, Head, Args...>
            > { return {type, functor}; }

            /**
             * Function object/pointer taking no arguments
             */
            template<class ReturnPolicy, class HeldType, class Functor>
            auto call_helper(
                const ClassWrapper<HeldType>* type,
                Functor functor,
                DummyTuple<>)
            -> std::enable_if_t<
                !std::is_member_function_pointer<Functor>::value,
                MethodBase<MethodOther, HeldType, Functor, ReturnPolicy>
            > { return {type, functor}; }

            /** Member function
             * \brief Exposes a functor as a method of the class
             */
            template<class ReturnPolicy, class HeldType, class Functor>
            Method<HeldType> wrap_functor(
                const ClassWrapper<HeldType>* type,
                const std::string& name,
                Functor functor)
            {
                using Sig = FunctionSignature<Functor>;
                return call_helper<ReturnPolicy>(
                    type,
                    functor,
                    typename Sig::argument_types_tag()
                ).method();
            }

            /**
             * Constructor, Functor
             */
            template<class HeldType, class Functor, class... Args, std::size_t... Indices>
            HeldType ctor_helper(
                Functor functor,
                const Object::Arguments& args,
                DummyTuple<Args...>,
                std::index_sequence<Indices...>)
            {
                return std::invoke(functor, args[Indices].cast<Args>()...);
            }

            /** Constructor, Functor
             * \brief Exposes a functor as a class constructor
             */
            template<class ReturnPolicy, class HeldType, class Functor>
            Constructor<HeldType> wrap_ctor(
                const ClassWrapper<HeldType>* type,
                Functor functor)
            {
                using Sig = FunctionSignature<Functor>;
                return {
                    typename Sig::argument_types_tag{},
                    [type, functor](const Object::Arguments& args) {
                        return type->parent_namespace().bind(
                            ctor_helper<HeldType>(
                                functor, args,
                                typename Sig::argument_types_tag{},
                                std::make_index_sequence<Sig::argument_count>()
                            ),
                            ReturnPolicy{}
                        );
                    }
                };
            }

            /**
             * Constructor
             */
            template<class HeldType, class... Args, std::size_t... Indices>
            HeldType raw_ctor_helper(
                const Object::Arguments& args,
                std::index_sequence<Indices...>)
            {
                return HeldType(args[Indices].cast<Args>()...);
            }

            /** Constructor
             * \brief Exposes a class constructor
             */
            template<class HeldType, class... Args>
            Constructor<HeldType> wrap_raw_ctor(const ClassWrapper<HeldType>* type)
            {
                return {
                    DummyTuple<Args...>(),
                    [type](const Object::Arguments& args) {
                        return type->parent_namespace().object(
                            raw_ctor_helper<HeldType, Args...>(
                                args,
                                std::make_index_sequence<sizeof...(Args)>()
                            )
                        );
                    }
                };
            }

        } // namespace function

    } // namespace detail

    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::add_readonly(
            const std::string& name, const T& value, ReturnPolicy)
        {
            getters[name] = detail::getter::wrap_getter<ReturnPolicy>(this, value);
            return *this;
        }


    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::fallback_getter(
            const T& functor, ReturnPolicy)
        {
            _fallback_getter = detail::getter::unreg_getter<ReturnPolicy>(this, functor);
            return *this;
        }

    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::add_method(
            const std::string& name, const T& value, ReturnPolicy)
        {
            methods.insert({
                name,
                detail::function::wrap_functor<ReturnPolicy>(this, name, value)
            });
            return *this;
        }

    template<class Class>
    template<class Read, class Write, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::add_readwrite(
            const std::string& name, const Read& read, const Write& write, ReturnPolicy)
        {
            getters[name] = detail::getter::wrap_getter<ReturnPolicy>(this, read);
            setters[name] = detail::setter::wrap_setter(this, write);
            return *this;
        }

    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::fallback_setter(const T& functor)
        {
            _fallback_setter = detail::setter::unreg_setter(this, functor);
            return *this;
        }

    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::constructor(
            const T& functor, ReturnPolicy)
    {
        _constructors.push_back(detail::function::wrap_ctor<ReturnPolicy>(this, functor));
        return *this;
    }

    template<class Class>
    template<class... Args>
        ClassWrapper<Class>& ClassWrapper<Class>::constructor()
    {
        _constructors.push_back(detail::function::wrap_raw_ctor<HeldType, Args...>(this));
        return *this;
    }

    template<class Class>
    Object ClassWrapper<Class>::make_object(const Object::Arguments& arguments) const
    {
        if ( _constructors.empty() )
            throw MemberNotFound("Class " + name() + " doesn't have a constructor");
        for ( const auto& ctor : _constructors )
            if ( ctor.can_call(arguments) )
                return ctor(arguments);
        throw MemberNotFound("No matching call to " + name() + " constructor");
    }

    template<class Class>
    template<class Target, class Functor, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::conversion(
            const Functor& functor, ReturnPolicy)
        {
            converters[typeid(Target)] = detail::getter::wrap_getter<ReturnPolicy>(this, functor);
            return *this;
        }

    template<class Class>
    template<class Functor, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::conversion(const Functor& functor, ReturnPolicy)
        {
            auto getter = detail::getter::wrap_getter<ReturnPolicy>(this, functor);
            using GetterType = decltype(getter);
            using Target = std::decay_t<typename GetterType::ReturnType>;
            converters[typeid(Target)] = std::move(getter);
            return *this;
        }

} // namespace wrapper

template<class T>
const T& Object::cast() const
{
    using Type = std::decay_t<T>;
    if ( auto ptr = dynamic_cast<wrapper::ObjectWrapper<Type>*>(value.get()) )
        return ptr->get();
    throw TypeError(
        "Object is of type " + value->type().name() + ", not "
        + value->type().parent_namespace().type_name<Type>()
    );
}

template<>
const Object& Object::cast<const Object&>() const
{
    return *this;
}

template<>
const Object& Object::cast<Object>() const
{
    return *this;
}

template<class T>
bool Object::has_type() const
{
    using Type = std::decay_t<T>;
    return dynamic_cast<wrapper::ObjectWrapper<Type>*>(value.get());
}


template<class T>
Object Object::converted() const
{
    using Type = std::decay_t<T>;
    if ( has_type<T>() )
        return *this;
    return value->converted(typeid(Type));
}

template<>
Object Object::converted<Object>() const
{
    return *this;
}

template<>
Object Object::converted<const Object&>() const
{
    return *this;
}

} // namespace scripting
} // namespace melanolib
#endif // MELANOLIB_SCRIPTING_OBJECT_HPP
