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
#include <algorithm>
#include <deque>

#include "melanolib/utils/c++-compat.hpp"
#include "melanolib/utils/functional.hpp"

namespace melanolib {
namespace scripting {

class TypeSystem;
class Object;

namespace wrapper {
    class TypeWrapper;
    using IteratorCallback = std::function<void (const Object&)>;
    using Arguments = std::vector<Object>;

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
         */
        virtual std::string to_string() const = 0;

        /**
         * \brief Returns a child object (or throws MemberNotFound)
         * \throws MemberNotFound
         */
        Object get_child(const Object& owner, const std::string& name) const;

        /**
         * \brief Sets a value on a child object
         * \throws MemberNotFound or TypeError
         * \pre args.size() == 2
         */
        Object set_child(const std::string& name, const Arguments& args);

        /**
         * \brief Calls a child function
         * \throws MemberNotFound
         */
        Object call_method(const std::string& name, const Arguments& args);

        /**
         * \brief Converts to an object of a different type
         */
        Object converted(const Object& owner, const std::type_info& type) const;

        virtual void iterate(const IteratorCallback& callback) = 0;

    private:
        const TypeWrapper* _type;
    };

    namespace detail {

        template<class T>
        auto get_reference(const T& value)
        ->std::enable_if_t<
            !std::is_pointer<T>::value &&
            !std::is_reference<T>::value,
            const T&>
        {
            return value;
        }

        template<class T>
        auto get_reference(std::remove_pointer_t<T>& value)
        ->std::enable_if_t<
            std::is_pointer<T>::value,
            T>
        {
            return &value;
        }

        template<class T>
        auto get_reference(T& value)
        ->std::enable_if_t<
            std::is_reference<T>::value,
            T&>
        {
            return value;
        }

        template<class T>
            using CastBase = std::decay_t<std::remove_pointer_t<std::decay_t<T>>>;

        template<class T>
            using CastResult = decltype(get_reference<T>(std::declval<CastBase<T>&>()));

        template<class T>
        CastResult<T> cast_helper(const Object& object, ValueWrapper* value);
    } // namespace detail
} // namespace wrapper

/**
 * \brief Base class for errors
 */
class Error : public std::runtime_error
{
public:
    explicit Error(const std::string& message)
        : runtime_error(message)
    {}
};

/**
 * \brief Exception thrown when trying to access a member that has not been exposed
 */
class MemberNotFound : public Error
{
public:
    using Error::Error;
};

/**
 * \brief Exception thrown when trying to access a type that has not been exposed
 */
class TypeError : public Error
{
public:
    using Error::Error;
};

/**
 * \brief Main interface to access objects
 */
class Object
{
public:
    using MemberPath = std::vector<std::string>;
    using Arguments = wrapper::Arguments;

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
        return value->get_child(*this, name);
    }

    /**
     * \brief Sets a direct attribute
     * \throw MemberNotFound or TypeError
     */
    Object set(const std::string& name, const Object& new_value) const
    {
        return value->set_child(name, {*this, new_value});
    }

    /**
     * \brief Invokes a member function
     * \throws MemberNotFound or TypeError
     */
    Object call(const std::string& method, Arguments args) const
    {
        args.insert(args.begin(), *this);
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
     * \brief Calls func with each element (for list-like objects)
     * \param Functor functor taking an Object as argument
     */
    template<class Functor>
    void iterate(Functor&& func) const
    {
        return value->iterate(std::forward<Functor>(func));
    }

    /**
     * \brief Cast to a reference of the contained type
     * \throws TypeError if the type doesn't match
     */
    template<class T>
    decltype(auto) cast() const
    {
        return wrapper::detail::cast_helper<T>(*this, value.get());
    }

    /**
     * \brief Whether it's safe to call cast<T>()
     */
    template<class T>
    bool has_type() const;

    /**
     * \brief Whether it's safe to call cast<T>() where typeid(T) == id
     */
    bool has_type(const std::type_index& id) const;

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

    bool has_value() const
    {
        return !!value;
    }

private:
    template<class Iter>
        Object get(Iter begin, Iter end) const
        {
            if ( begin >= end )
                return *this;
            return value->get_child(*this, *begin).get(begin+1, end);
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
 * \brief Object used to store values, by default stores a copy or a pointer
 *
 * Specialize if a different behaviour is required
 */
template<class T, bool NoByValue = std::is_abstract<T>::value>
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

template<class T>
    struct ValueHolder<T, true>
{
    ValueHolder(T* pointer)
        : holder(pointer)
    {}

    ValueHolder(const Ref<T>& reference)
        : holder(&reference.get())
    {}

    const T& get() const
    {
        return *holder;
    }

    T& get()
    {
        return *holder;
    }

    T* holder;
};

namespace wrapper {
    template<class Class> class ClassWrapper;
    namespace detail {
        using FunctorBase = std::function<Object(const TypeWrapper*, const Object::Arguments&)>;

        template<int can_skip>
        class Overloadable
        {
        public:
            using TypeList = std::vector<std::type_index>;

            template<class FunctorT, class... Args>
            Overloadable(DummyTuple<Args...>, FunctorT&& functor)
                : functor(std::forward<FunctorT>(functor)),
                  types({std::type_index(typeid(std::remove_pointer_t<Args>))...})
            {}

            bool can_call(const Object::Arguments& args) const
            {
                if ( args.size() < types.size() )
                    return false;

                auto args_iter = args.begin();

                if ( can_skip && args.size() == types.size() + can_skip )
                    args_iter += can_skip;
                else if ( args.size() != types.size() )
                    return false;

                for ( const auto& type : types )
                {
                    if ( !args_iter->has_type(type) )
                        return false;
                    ++args_iter;
                }
                return true;
            }

            /**
             * \pre can_call(args)
             */
            Object operator()(const TypeWrapper* type, const Object::Arguments& args) const
            {
                if ( args.size() != types.size() && args.size() != types.size() + can_skip )
                    throw TypeError("Wrong number of arguments");
                return functor(type, args);
            }
        private:
            FunctorBase functor;
            TypeList types;
        };

        template<int minargs, int maxargs = minargs>
        class LimitArgs
        {
        public:
            template<class... Args>
            LimitArgs(Args&&... functor)
                : functor(std::forward<Args>(functor)...)
            {}

            Object operator()(const TypeWrapper* type, const Object::Arguments& args) const
            {
                if ( args.size() < minargs || args.size() > maxargs )
                    throw TypeError("Wrong number of arguments");

                return functor(type, args);
            }

            explicit operator bool() const
            {
                return bool(functor);
            }

        private:
            FunctorBase functor;
        };

        using Method = Overloadable<1>;
        using MethodMap = std::unordered_multimap<std::string, Method>;

        using Setter = LimitArgs<1, 2>;
        using SetterMap = std::unordered_map<std::string, Setter>;
        using UnregSetter = LimitArgs<2, 3>;

        using Constructor = Overloadable<0>;
        using ConstructorList = std::vector<Constructor>;


        using Getter = LimitArgs<0, 1>;
        using GetterMap = std::unordered_map<std::string, Getter>;
        using UnregGetter = LimitArgs<1, 2>;

        using ConverterMap = std::unordered_map<std::type_index, Getter>;

        template<class HeldType>
            using Iterator = std::function<void(
                const ClassWrapper<HeldType>*,
                HeldType&,
                const IteratorCallback&)>;

        template<class HeldType>
            using Stringizer = std::function<std::string(const HeldType&)>;

    } // namespace detail

    /**
     * \brief Wrapper around a type (erasure base)
     */
    class TypeWrapper
    {
    public:
        TypeWrapper(std::string&& name, const TypeSystem& type_system)
            : _name(std::move(name)),
            _type_system(&type_system)
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

        const TypeSystem& type_system() const
        {
            return *_type_system;
        }

        /**
         * \brief Changes this type's name
         */
        void rename(const std::string& name)
        {
            _name = name;
        }

        /**
         * \brief Creates a copy pf the run-time type
         */
        virtual std::unique_ptr<TypeWrapper> clone() const = 0;

        /**
         * \brief Searches the inheritance tree (Breadth-first)
         *        to check if it inherits the given type
         */
        bool inherits(const TypeWrapper& wrapper) const
        {
            std::deque<const TypeWrapper*> wrappers;
            wrappers.push_back(this);
            while ( !wrappers.empty() )
            {
                auto top = wrappers.front();
                wrappers.pop_front();
                if ( top == &wrapper )
                    return true;
                wrappers.insert(wrappers.end(), top->supertypes.begin(), top->supertypes.end());
            }
            return false;
        }

        /**
         * \brief Returns an attribute of the passed object
         * \throws MemberNotFound if \p name is not something registered
         * with add_readonly or add_readwrite
         */
        Object get_value(const Object& owner, const std::string& attrname) const
        {
            auto iter = getters.find(attrname);
            if ( iter == getters.end() )
            {
                if ( _fallback_getter )
                    return _fallback_getter(this, {owner, make_foreign_object(attrname)});
                throw MemberNotFound("\"" + attrname + "\" is not a member of " + name());
            }
            return iter->second(this, {owner});
        }

        /**
         * \brief Sets the value of an attribute
         * \throws MemberNotFound if \p name is not something registered
         * with add_readwrite
         * \throws TypeError if \p args can't be properly converted
         * \pre args.size() == 2
         */
        Object set_value(const std::string& attrname, const Object::Arguments& args) const
        {
            auto iter = setters.find(attrname);
            if ( iter == setters.end() )
            {
                if ( _fallback_setter )
                {
                    return _fallback_setter(this, {args[0], make_foreign_object(attrname), args[1]});
                }
                throw MemberNotFound("\"" + attrname + "\" is not a writable member of " + name());
            }
            return iter->second(this, args);
        }

        /**
         * \brief Returns an attribute of the passed object
         * \throws MemberNotFound if \p name is not something registered
         * with one of the add_readonly() overloads
         */
        Object call_method(
            const std::string& method,
            const Object::Arguments& arguments) const
        {
            auto range = methods.equal_range(method);
            if ( range.first == range.second )
                throw MemberNotFound("\"" + method + "\" is not a member function of " + name());

            for ( auto it = range.first; it != range.second; ++it )
                if ( it->second.can_call(arguments) )
                    return it->second(this, arguments);
            throw MemberNotFound("No matching overload of \"" + method + "\" in " + name());
        }

        /**
         * \brief Calls a dynamic constructor
         */
        Object make_object(const Object::Arguments& arguments) const;

        /**
         * \brief Returns an Object with a converted type
         * \throws MemberNotFound if \p name is not something registered
         * with conversion()
         */
        Object convert(const Object& owner, const std::type_info& type) const;

    protected:
        void inherit(const TypeWrapper& parent)
        {
            supertypes.push_back(&parent);
        }


        /**
         * \brief String conversion for values that can be converted implicitly to a string
         *        using streams.
         */
        template<class T>
        std::enable_if_t<
            std::is_convertible<T, std::string>::value,
            std::string>
            value_to_string(const T& value) const
        {
            return value;
        }

        /**
         * \brief String conversion for values that can be converted to a string
         *        using streams.
         */
        template<class T>
        std::enable_if_t<
            StreamInsertable<T>::value &&
            !std::is_convertible<T, std::string>::value,
            std::string>
            value_to_string(const T& value) const
        {
            std::ostringstream stream;
            stream << value;
            return stream.str();
        }

        /**
         * \brief String conversion for values that can't be converted to a string
         */
        template<class T>
        std::enable_if_t<
            !StreamInsertable<T>::value &&
            !std::is_convertible<T, std::string>::value,
            std::string>
            value_to_string(const T&) const
        {
            return name();
        }

    private:
        template<class T>
        Object make_foreign_object(const T& obj) const;

        /**
         * \brief Moves to a different namespace
         */
        void migrate_to(const TypeSystem& type_system)
        {
            _type_system = &type_system;
        }

        std::string _name;
        const TypeSystem* _type_system;
        std::vector<const TypeWrapper*> supertypes;

    protected:
        detail::GetterMap getters;
        detail::UnregGetter _fallback_getter;
        detail::MethodMap methods;
        detail::SetterMap setters;
        detail::UnregSetter _fallback_setter;
        detail::ConstructorList _constructors;
        detail::ConverterMap converters;

        friend TypeSystem;
    };

    /**
     * \brief Wrapper around a class
     */
    template<class Class>
        class ClassWrapper : public TypeWrapper
    {
    public:
        using HeldType = Class;

        ClassWrapper(std::string name, const TypeSystem& type_system)
            : TypeWrapper(std::move(name), type_system)
        {}

        /**
         * \brief Exposes an attribute
         * \tparam T can be:
         * * Any callable that can be invoked with an optional pointer or reference to HeldType
         * * Any other object of a type registered to the parent namespace
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         * \note If you want to wrap a reference to an external object, you'll need
         * to ensure \p value has a type wrapped in Ref (eg: by calling wrap_reference)
         */
        template<class T, class ReturnPolicy = CopyPolicy>
            ClassWrapper& add_readonly(const std::string& name, const T& value, ReturnPolicy = {});


        /**
         * \brief Sets a fallback functions used to get additional unregistered
         *        attributes
         * \tparam T can be:
         * * Any callable that can be invoked with an optional pointer or
         *   reference to HeldType and a string
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         */
        template<class T, class ReturnPolicy = CopyPolicy>
            ClassWrapper& fallback_getter(const T& functor, ReturnPolicy = {});

        /**
         * \brief Exposes an attribute
         * \tparam Read can be:
         * * Any callable that can be invoked with an optional pointer or reference to HeldType
         * * Any other object of a type registered to the parent namespace
         * \tparam Write can be:
         * * Any callable that can be invoked with an optional pointer or
         *   reference to HeldType and an argument of any type registered to the typesystem
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         * \note If you want to wrap a reference to an external object, you'll need
         * to ensure \p value has a type wrapped in Ref (eg: by calling wrap_reference)
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
         * * Any callable that can be invoked with an optional reference or
         *   pointer to HeldType, a string, and an additional argument
         */
        template<class T>
            ClassWrapper& fallback_setter(const T& functor);

        /**
         * \brief Exposes a member function
         * \tparam T can be:
         * * Any callable, if it takes as first argument a a reference or
         *   pointer to HeldType, the current object will be passed
         * * Any other object of a type registered to the parent namespace
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         * \note If you want to wrap a reference to an external object, you'll need
         * to ensure \p value has a type wrapped in Ref (eg: by calling wrap_reference)
         */
        template<class T, class ReturnPolicy = CopyPolicy>
            ClassWrapper& add_method(const std::string& name, const T& value,
                                     ReturnPolicy = {});

        /**
         * \brief Sets the function used as a constructor
         * \tparam T can be:
         * * Any callable object returning an object, pointer or reference of HeldType
         * * An object of type HeldType
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         * \note If you want to wrap a reference to an external object, you'll need
         * to ensure \p value has a type wrapped in Ref (eg: by calling wrap_reference)
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
         * \tparam Target The type this conversion converts to
         * \tparam Functor can be:
         * * Any callable that can be invoked with an optional pointer or
         *   reference to HeldType that returns a value convertible (in C++)
         *   to \p Target
         * * Any other object of a type registered to the parent namespace
         *   convertible (in C++) to \p Target
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the returned value
         * \note If you want to wrap a reference to an external object, you'll need
         * to ensure \p value has a type wrapped in Ref (eg: by calling wrap_reference)
         */
        template<class Target, class Functor, class ReturnPolicy = CopyPolicy>
            ClassWrapper& conversion(const Functor& functor, ReturnPolicy = {});

        /**
         * \brief Exposes a conversion operator
         *
         * The target type is deduced from the result of a call to \p Functor
         */
        template<class Functor, class ReturnPolicy = CopyPolicy>
            ClassWrapper& conversion(const Functor& functor, ReturnPolicy = {});

        /**
         * \brief Exposes a function to be used for to_string
         * \tparam Functor can be:
         * * A pointer to a data member of HeldType
         * * A pointer to a member function of HeldType taking no arguments
         * * Any function object taking a const HeldType& argument
         */
        template<class Functor, class ReturnPolicy = CopyPolicy>
            ClassWrapper& string_conversion(const Functor& functor)
            {
                stringizer = functor;
                return *this;
            }

        /**
         * \brief Makes a type iterable by using two functor
         * \tparam Begin An object invokable with a reference to HeldType
         * \tparam End An object invokable with a reference to HeldType
         * \tparam Filter A functor invokable with the result of a dereferenced iterator
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the values returned by \p filter
         */
        template<class Begin, class End, class Filter = Identity, class ReturnPolicy = CopyPolicy>
            ClassWrapper& make_iterable(
                const Begin& begin,
                const End& end,
                const Filter& filter = {},
                ReturnPolicy = {})
        {
            iterator = [begin, end, filter](
                const ClassWrapper* type,
                HeldType& value,
                const std::function<void (const Object&)>& callback )
            {
                auto iter = std::invoke(begin, value);
                auto enditer = std::invoke(end, value);
                for ( ; iter != enditer; ++iter )
                {
                    callback(type->type_system().bind(filter(*iter), ReturnPolicy{}));
                }
            };
            return *this;
        }

        /**
         * \brief Makes a type iterable by using std::begin and std::end
         * \tparam ReturnPolicy CopyPolicy or WrapReferencePolicy, to determine
         *                      how to bind the values dereferenced from the iterators
         */
        template<class ReturnPolicy = CopyPolicy>
            ClassWrapper& make_iterable(ReturnPolicy = {})
        {
            return make_iterable(Begin{}, End{}, Identity{}, ReturnPolicy{});
        }

        /**
         * \brief Exposes inheritance
         * \tparam T Type registered on the same type system
         */
        template<class T>
        ClassWrapper& inherit();
        ClassWrapper& inherit(const std::string& type_name);

        std::type_index type_index() const noexcept override
        {
            return typeid(HeldType);
        }

        std::unique_ptr<TypeWrapper> clone() const override
        {
            return std::make_unique<ClassWrapper>(*this);
        }

        void iterate(HeldType& owner, const IteratorCallback& callback) const
        {
            if ( !iterator )
                throw MemberNotFound("Cannot iterate " + name());
            iterator(this, owner, callback);
        }

        std::string to_string(const HeldType& value) const
        {
            if ( stringizer )
                return stringizer(value);
            return value_to_string(value);
        }

    private:
        detail::Iterator<HeldType> iterator;
        detail::Stringizer<HeldType> stringizer;
    };

} // namespace wrapper

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
            return class_wrapper().to_string(value.get());
        }

        const ClassWrapper<Class>& class_wrapper() const
        {
            return static_cast<const ClassWrapper<Class>&>(type());
        }

        const Class& get() const
        {
            return value.get();
        }

        Class& get()
        {
            return value.get();
        }

        void iterate(const IteratorCallback& callback) override
        {
            class_wrapper().iterate(value.get(), callback);
        }

    private:
        ValueHolder<Class> value;
    };


    inline Object ValueWrapper::get_child(const Object& owner, const std::string& name) const
    {
        return type().get_value(owner, name);
    }

    inline Object ValueWrapper::set_child(const std::string& name, const Object::Arguments& args)
    {
        return type().set_value(name, args);
    }

    inline Object ValueWrapper::call_method(const std::string& name, const Object::Arguments& args)
    {
        return type().call_method(name, args);
    }

    inline Object ValueWrapper::converted(const Object& owner, const std::type_info& type_info) const
    {
        return type().convert(owner, type_info);
    }

} // namespace wrapper

/**
 * \brief Template used to register typesm
 * Specialize auto_register to add default attributes
 */
template<class Type>
struct Registrar
{
    using TypeWrapper = wrapper::ClassWrapper<Type>;
    using Pointer = std::unique_ptr<TypeWrapper>;

    static Pointer create_wrapper(const std::string& name, TypeSystem& ns)
    {
        auto ptr = std::make_unique<TypeWrapper>(name, ns);
        auto_register(*ptr, ns);
        return ptr;
    }

    static void auto_register(TypeWrapper& type, TypeSystem& ns)
    {
    }
};

/**
 * \brief Type registry
 */
class TypeSystem
{
public:
    /**
     * \brief Registers a type
     * \returns The registered class wrapper
     */
    template<class Type>
    typename Registrar<Type>::TypeWrapper& register_type(const std::string& name)
    {
        auto ptr = Registrar<Type>::create_wrapper(name, *this);
        auto& ref = *ptr;
        classes[ptr->type_index()] = std::move(ptr);
        return ref;
    }

    /**
     * \brief Registers a type using a default name
     * \returns The registered class wrapper
     */
    template<class Type>
    typename Registrar<Type>::TypeWrapper& register_type()
    {
        return register_type<Type>(typeid(Type).name());
    }

    /**
     * \brief Registers the type if it doesn't already exist
     */
    template<class Type>
    void ensure_type(const std::string& name = "")
    {
        if ( !classes.count(typeid(Type)) )
            register_type<Type>(name.empty() ? name : typeid(Type).name());
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
     * \brief Creates an object wrapper around the value
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object bind(const Ref<Class>& value, CopyPolicy) const
    {
        return object(value.get());
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
     * \brief Creates an object wrapper around the reference
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object bind(const Ref<Class>& value, WrapReferencePolicy) const
    {
        return reference(value.get());
    }

    /**
     * \brief Creates an object wrapper around the reference
     * \throws TypeError if \p Class has not been registered with register_type
     */
    template<class Class>
    Object bind(Ref<Class>& value, WrapReferencePolicy) const
    {
        return reference(value.get());
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
    Object object(const std::string& type_name, const Object::Arguments& args) const
    {
        const auto& p = find_type(type_name);
        return p.second->make_object(args);
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

    /**
     * \brief Import a type definition from a different namespace
     * \param source Type system to copy the type from
     * \param type_name Name of the type to copy from \p source
     * \param clear_inheritance Whether to clear supertype info.
     * If \b false and the type has supertypes, an exception will be thrown
     */
    wrapper::TypeWrapper& import_type(
        const TypeSystem& source,
        const std::string& type_name,
        bool clear_inheritance = false)
    {
        return import_single(source.find_type(type_name), clear_inheritance);
    }

    wrapper::TypeWrapper& import_type(
        const TypeSystem& source,
        const std::type_info& type_info,
        bool clear_inheritance = false)
    {
        return import_single(source.find_type(type_info), clear_inheritance);
    }

    template<class T>
    wrapper::TypeWrapper& import_type(
        const TypeSystem& source,
        bool clear_inheritance = false)
    {
        return import_type(source, typeid(T), clear_inheritance);
    }

    /**
     * \brief Import all type definitions from a different namespace
     */
    void import(const TypeSystem& source)
    {
        std::unordered_map<const wrapper::TypeWrapper*, const wrapper::TypeWrapper*> transform;
        for ( const auto& p : source.classes )
        {
            auto ptr = p.second->clone();
            ptr->migrate_to(*this);
            transform[p.second.get()] = ptr.get();
            classes[p.first] = std::move(ptr);
        }

        for ( auto& type : classes )
            std::transform(
                type.second->supertypes.begin(),
                type.second->supertypes.end(),
                type.second->supertypes.begin(),
                [&transform](const wrapper::TypeWrapper* from)
                { return transform[from]; }
            );
    }

    /**
     * \brief Returns a reference to the wrapper for the given type
     * \throws TypeError if the type has not been registered
     */
    const wrapper::TypeWrapper& wrapper_for(const std::string& type_name) const
    {
        return *find_type(type_name).second;
    }

    template<class T>
        const wrapper::TypeWrapper& wrapper_for() const
        {
            return *find_type(typeid(T)).second;
        }

private:
    using TypeMap = std::unordered_map<std::type_index, std::unique_ptr<wrapper::TypeWrapper>>;
    using TypeItem = TypeMap::value_type;

    template<class Class, class Ctor>
    Object build_object(Ctor&& ctor_arg) const
    {
        auto iter = classes.find(typeid(Class));
        if ( iter == classes.end() )
            throw TypeError("Unregistered type");
        return Object(std::make_shared<wrapper::ObjectWrapper<Class>>(
            std::forward<Ctor>(ctor_arg),
            static_cast<wrapper::ClassWrapper<Class>*>(iter->second.get())
        ));
    }

    const TypeItem& find_type(const std::string& type_name) const
    {
        for ( const TypeItem& p : classes )
            if ( p.second->name() == type_name )
                return p;
        throw TypeError("Unregistered type: " + type_name);
    }

    const TypeItem& find_type(const std::type_info& type_info) const
    {
        auto iter = classes.find(type_info);
        if ( iter == classes.end() )
            throw TypeError("Unregistered type");
        return *iter;
    }

    wrapper::TypeWrapper& import_single(const TypeItem& pair, bool clear_inheritance)
    {
        if ( !pair.second->supertypes.empty() && !clear_inheritance )
            throw TypeError("Cannot import single type as it has supertypes");
        auto& type_ptr = classes[pair.first] = pair.second->clone();
        type_ptr->migrate_to(*this);
        type_ptr->supertypes.clear();
        return *type_ptr;
    }

    TypeMap classes;
};

namespace wrapper {

    namespace detail {

        template<class Functor, class ReturnPolicy, class Ret, class... Args>
        class FunctorWrapper
        {
        public:
            using return_type = Ret;

            Functor functor;

            Object operator()(
                const TypeWrapper* type,
                const Object::Arguments& args) const
            {
                return call_and_bind(type, args, std::make_index_sequence<sizeof...(Args)>{});
            }

            Object operator()(
                const TypeWrapper* type,
                const Object& arg) const
            {
                return (*this)(type, Object::Arguments(1, arg));
            }

            Method method() const
            {
                return {DummyTuple<Args...>{}, *this};
            }

        private:
            /*
             * Invokable
             */
            template<class... Args2, std::size_t... Indices>
            auto invoke(
                const Object::Arguments& args,
                std::index_sequence<Indices...> seq,
                const FunctorWrapper*
            ) const
                -> decltype(std::__invoke(functor, args[Indices].cast<Args2>()...))
            {
                auto iter = args.begin();
                if ( sizeof...(Args2) < args.size() )
                    ++iter;
                return std::__invoke(functor, iter[Indices].cast<Args2>()...);
            }

            /*
             * Not ivokable
             */
            template<class... Args2, std::size_t... Indices>
            decltype(auto) invoke(
                const Object::Arguments& args,
                std::index_sequence<Indices...>,
                const void*
            ) const
            {
                return functor;
            }

            template<std::size_t... Indices>
            auto call_and_bind(
                const TypeWrapper* type,
                const Object::Arguments& args,
                std::index_sequence<Indices...> indices) const
            -> std::enable_if_t<
                !std::is_void<decltype(invoke<Args...>(args, indices, this))>::value,
                Object>
            {
                return type->type_system().bind(
                    invoke<Args...>(args, indices, this),
                    ReturnPolicy{}
                );
            }

            template<std::size_t... Indices>
            auto call_and_bind(
                const TypeWrapper* type,
                const Object::Arguments& args,
                std::index_sequence<Indices...> indices) const
            -> std::enable_if_t<
                std::is_void<decltype(invoke<Args...>(args, indices, this))>::value,
                Object>
            {
                invoke<Args...>(args, indices, this);
                return Object({});
            }
        };

        template<class ReturnPolicy, class Ret, class Functor, class... Args>
        FunctorWrapper<Functor, ReturnPolicy, Ret, Args...>
            resolve_method_arguments(Functor functor, DummyTuple<Args...>)
        { return {functor}; }

        template<class ReturnPolicy, class Functor>
        Method wrap_method(Functor functor)
        {
            using Sig = FunctionSignature<Functor>;
            return {
                typename Sig::invoke_types_tag(),
                resolve_method_arguments<ReturnPolicy, typename Sig::return_type>(
                    functor,
                    typename Sig::invoke_types_tag()
                )
            };
        }

        template<class ReturnPolicy, class Functor>
        Constructor wrap_ctor(Functor functor)
        {
            using Sig = FunctionSignature<Functor>;
            return {
                typename Sig::invoke_types_tag(),
                resolve_method_arguments<ReturnPolicy, typename Sig::return_type>(
                    functor,
                    typename Sig::invoke_types_tag()
                )
            };
        }

        /**
         * Constructor
         */
        template<class Class, class... Args, std::size_t... Indices>
        Class raw_ctor_helper(
            const Object::Arguments& args,
            std::index_sequence<Indices...>)
        {
            return Class(args[Indices].cast<Args>()...);
        }

        /** Constructor
            * \brief Exposes a class constructor
            */
        template<class Class, class... Args>
        Constructor wrap_raw_ctor()
        {
            return {
                DummyTuple<Args...>(),
                [](const TypeWrapper* type,
                        const Object::Arguments& args) {
                    return type->type_system().object(
                        raw_ctor_helper<Class, Args...>(
                            args,
                            std::make_index_sequence<sizeof...(Args)>()
                        )
                    );
                }
            };
        }

        template<class T>
        struct MakeSetter
        {
            using type = T;
        };

        template<class Class, class Ret, class Args>
        struct MakeSetter<Ret (Class::*)(Args)>
        {
            using type = Ret (Class::*)(Args);
        };

        template<class Class, class Type>
        struct MakeSetter<Type Class::*>
        {
            using type = MakeSetter;

            MakeSetter(Type Class::* pointer)
                : pointer(pointer) {}

            decltype(auto) operator()(Class& obj, const Type& value) const
            {
                return obj.*pointer = value;
            }

            Type Class::* pointer;
        };

        template<class ReturnPolicy, class Functor>
        Setter wrap_setter(Functor functor)
        {
            using Setter = typename MakeSetter<Functor>::type;
            using Sig = FunctionSignature<Setter>;
            return resolve_method_arguments<ReturnPolicy, typename Sig::return_type>(
                Setter(functor),
                typename Sig::invoke_types_tag()
            );
        }

        template<class ReturnPolicy, class Functor>
        auto wrap_functor(Functor functor)
        {
            using Sig = FunctionSignature<Functor>;
            return resolve_method_arguments<ReturnPolicy, typename Sig::return_type>(
                functor,
                typename Sig::invoke_types_tag()
            );
        }

    } // namespace detail

    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::add_readonly(
            const std::string& name, const T& value, ReturnPolicy)
        {
            getters[name] = detail::wrap_functor<ReturnPolicy>(value);
            return *this;
        }


    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::fallback_getter(
            const T& functor, ReturnPolicy)
        {
            _fallback_getter = detail::wrap_functor<ReturnPolicy>(functor);
            return *this;
        }

    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::add_method(
            const std::string& name, const T& value, ReturnPolicy)
        {
            methods.insert({
                name,
                detail::wrap_method<ReturnPolicy>(value)
            });
            return *this;
        }

    template<class Class>
    template<class Read, class Write, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::add_readwrite(
            const std::string& name, const Read& read, const Write& write, ReturnPolicy)
        {
            getters[name] = detail::wrap_functor<ReturnPolicy>(read);
            setters.insert({
                name,
                detail::wrap_setter<ReturnPolicy>(write)
            });
            return *this;
        }

    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::fallback_setter(const T& functor)
        {
            _fallback_setter = detail::wrap_functor<CopyPolicy>(functor);
            return *this;
        }

    template<class Class>
    template<class T, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::constructor(
            const T& functor, ReturnPolicy)
    {
        _constructors.push_back(detail::wrap_ctor<ReturnPolicy>(functor));
        return *this;
    }

    template<class Class>
    template<class... Args>
        ClassWrapper<Class>& ClassWrapper<Class>::constructor()
    {
        _constructors.push_back(detail::wrap_raw_ctor<HeldType, Args...>());
        return *this;
    }

    template<class T>
    Object TypeWrapper::make_foreign_object(const T& obj) const
    {
        return type_system().object(obj);
    }

    Object TypeWrapper::make_object(const Object::Arguments& arguments) const
    {
        if ( _constructors.empty() )
            throw MemberNotFound("Class " + name() + " doesn't have a constructor");
        for ( const auto& ctor : _constructors )
            if ( ctor.can_call(arguments) )
                return ctor(this, arguments);
        throw MemberNotFound("No matching call to " + name() + " constructor");
    }

    Object TypeWrapper::convert(const Object& owner, const std::type_info& type) const
    {
        auto iter = converters.find(type);
        if ( iter == converters.end() )
        {
            throw MemberNotFound("Cannot convert " + name() + " to " +
                type_system().type_name(type));
        }
        return iter->second(this, {owner});
    }

    template<class Class>
    template<class Target, class Functor, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::conversion(
            const Functor& functor, ReturnPolicy)
        {
            converters[typeid(Target)] = detail::wrap_functor<ReturnPolicy>(functor);
            return *this;
        }

    template<class Class>
    template<class Functor, class ReturnPolicy>
        ClassWrapper<Class>& ClassWrapper<Class>::conversion(const Functor& functor, ReturnPolicy)
        {
            auto getter = detail::wrap_functor<ReturnPolicy>(functor);
            using GetterType = decltype(getter);
            using Target = std::decay_t<typename GetterType::return_type>;
            converters[typeid(Target)] = std::move(getter);
            return *this;
        }

    template<class Class>
    template<class T>
        ClassWrapper<Class>& ClassWrapper<Class>::inherit()
        {
            TypeWrapper::inherit(type_system().template wrapper_for<T>());
        }

    template<class Class>
        ClassWrapper<Class>& ClassWrapper<Class>::inherit(const std::string& type_name)
        {
            TypeWrapper::inherit(type_system().wrapper_for(type_name));
        }


    namespace detail {
        template<class Type>
        struct CastTemplate
        {
            template<class T>
            static decltype(auto) cast_helper(const Object&, ValueWrapper* value)
            {
                if ( auto ptr = dynamic_cast<wrapper::ObjectWrapper<Type>*>(value) )
                    return get_reference<T>(ptr->get());

                throw TypeError(
                    "Object is of type " + value->type().name() + ", not "
                    + value->type().type_system().type_name<Type>()
                );
            }
        };

        template<>
        struct CastTemplate<Object>
        {
            template<class T>
            static decltype(auto) cast_helper(const Object& object, ValueWrapper*)
            {
                    return get_reference<T>(object);
            }
        };

        template<class T>
        decltype(get_reference<T>(std::declval<CastBase<T>&>()))
        cast_helper(const Object& object, ValueWrapper* value)
        {
            return CastTemplate<CastBase<T>>::template cast_helper<T>(object, value);
        }
    } // namespace detail
} // namespace wrapper

template<class T>
bool Object::has_type() const
{
    using Type = std::decay_t<std::remove_pointer_t<std::decay_t<T>>>;
    return dynamic_cast<wrapper::ObjectWrapper<Type>*>(value.get());
}

template<>
inline bool Object::has_type<const Object&>() const
{
    return true;
}

template<>
inline bool Object::has_type<Object>() const
{
    return true;
}


template<class T>
Object Object::converted() const
{
    using Type = std::decay_t<T>;
    if ( has_type<T>() )
        return *this;
    return value->converted(*this, typeid(Type));
}

template<>
inline Object Object::converted<Object>() const
{
    return *this;
}

template<>
inline Object Object::converted<const Object&>() const
{
    return *this;
}

inline bool Object::has_type(const std::type_index& id) const
{
    if ( !has_value() )
        return false;
    if ( id == typeid(Object) )
        return true;
    return id == type().type_index();
}

/**
 * \brief Simple wrappable type that exposes arbitraty attributes
 */
class SimpleType
{
public:
    Object get(const std::string& name) const
    {
        auto iter = attributes.find(name);
        if ( iter != attributes.end() )
            return iter->second;
        throw MemberNotFound(name);
    }

    void set(const std::string& name, const Object& value)
    {
        attributes.insert({name, value});
    }

private:
    std::unordered_map<std::string, Object> attributes;
};

template<>
inline void Registrar<SimpleType>::auto_register(TypeWrapper& type, TypeSystem& ns)
{
    type.fallback_getter(&SimpleType::get);
    type.fallback_setter(&SimpleType::set);
}

} // namespace scripting
} // namespace melanolib
#endif // MELANOLIB_SCRIPTING_OBJECT_HPP
