#ifndef VARIANT_H
#define VARIANT_H

#include <type_traits>
#include <iostream>
#include <new>
#include <utility>
#include <string>

namespace vr {

using std::cerr;
using std::endl;

template<typename... Ts> struct my_variant;

namespace details {

    namespace utility {

        template<size_t a, size_t... b>
        struct __variant_max {};

        template<size_t sz>
        struct __variant_max<sz> {
            static const size_t value = sz;
        };

        template<size_t sz1, size_t sz2, size_t... rest>
        struct __variant_max<sz1, sz2, rest...> {
            static const size_t value = sz1 > sz2 ? __variant_max<sz1, rest...>::value : __variant_max<sz2, rest...>::value;
        };

        template<typename... Ts>
        struct __first_type;

        template<typename T, typename... Ts>
        struct __first_type<T, Ts...> {
            using type = T;
        };

        template<typename T>
        struct is_my_variant {
            static const bool value = false;
        };

        template<template <typename... > typename C, typename... Ts>
        struct is_my_variant<C<Ts...>> {
            static const bool value = std::is_same<C<Ts...>, my_variant<Ts...>>::value;
        };


    }

    namespace visit_details {

        template<typename Visitor,	typename T>
        using __visit_type_for_element = decltype(std::declval<Visitor>()(std::declval<T>()));

        struct __empty_result_type;

        template<typename Result, typename Visitor, typename... Ts>
        struct __visit_result_impl {
            using type = Result;
        };

        template<typename Visitor, typename... Ts>
        struct __visit_result_impl<__empty_result_type, Visitor, Ts...> {
            using type = typename std::common_type<__visit_type_for_element<Visitor, Ts>...>::type;
        };
    }
}

template<typename... Ts>
using max_types_size = details::utility::__variant_max<sizeof(Ts)...>;

template<typename... Ts>
using max_types_align = details::utility::__variant_max<alignof(Ts)...>;

template<typename... Ts>
using first_type = details::utility::__first_type<Ts...>;

template<typename Result, typename Visitor, typename...	Ts>
using visit_result = typename details::visit_details::__visit_result_impl<Result, Visitor, Ts...>::type;

template<typename... Ts>
struct index_of_type {
    static constexpr size_t value = 0;
};

template<typename T, typename F, typename... Rest>
struct index_of_type<T, F, Rest...> {
    static constexpr size_t value = std::integral_constant<size_t, std::is_same<T, F>::value ? 0 : index_of_type<T, Rest...>::value + 1> {};
};

template<typename Result, typename Variant, typename Visitor, typename First, typename... Rest>
Result visit_impl(Variant&& variant, Visitor&& visitor, std::tuple<First, Rest...>) {
    if (variant.template holds_alternative<First>()) {
        return static_cast<Result>(std::forward<Visitor>(visitor)(std::forward<Variant>(variant).template get<First>()));
    } else if constexpr (sizeof...(Rest) > 0) {
        return	visit_impl<Result>(std::forward<Variant>(variant), std::forward<Visitor>(visitor), std::tuple<Rest...>());
    }
}

template<typename... Ts>
struct my_variant_storage {

    size_t get_index() const { return _id; }
    void set_index(int id) {_id = id; }
    void* get_storage_pointer() {return &_data; }
    void const* get_storage_pointer() const {return &_data; }
    template<typename T>
    T* get_storage_pointer() {return std::launder(reinterpret_cast<T*>(&_data)); }
    template<typename T>
    T const* get_storage_pointer() const {return std::launder(reinterpret_cast<T const*>(&_data)); }
    //https://miyuki.github.io/2016/10/21/std-launder.html

private:

    static constexpr size_t _size = max_types_size<Ts...>::value;
    static constexpr size_t _align = max_types_align<Ts...>::value;
    using type = typename std::aligned_storage<_size, _align>::type;

    type _data;
    size_t _id;

};

template<typename T, typename... Ts>
struct my_variant_helper {

    using Derived = my_variant<Ts...>;

    my_variant_helper() {}

    my_variant_helper(T const& other) {
        new (get_derived().get_storage_pointer()) T(other);
        get_derived().set_index(id);
    }

    my_variant_helper(const T&& other) {
        new (get_derived().get_storage_pointer()) T(std::move(other));
        get_derived().set_index(id);
    }

    bool destroy() {
        if (get_derived().get_index() == id) {
            get_derived().template get_storage_pointer<T>()->~T();
            return true;
        }
        return false;
    }

    Derived& operator =(T const& other) {
        if (get_derived().get_index() == id) {
            *get_derived().template get_storage_pointer<T>() = other;
        } else {
            get_derived().destroy();
            new (get_derived().get_storage_pointer()) T(other);
            get_derived().set_index(id);
        }
        return get_derived();
    }

    Derived& operator =(T&& other) {
        if (get_derived().get_index() == id) {
            *get_derived().template get_storage_pointer<T>() = std::move(other);
        } else {
            get_derived().destroy();
            new (get_derived().get_storage_pointer()) T(std::move(other));
            get_derived().set_index(id);
        }
        return get_derived();
    }

protected:

    constexpr static size_t id = index_of_type<T, Ts...>::value;

private:

    Derived& get_derived() {return *static_cast<Derived*>(this); }
    Derived const& get_derived() const {return *static_cast<Derived const*>(this); }

};

template<typename... Ts>
struct my_variant : private my_variant_storage<Ts...>, private my_variant_helper<Ts, Ts...>... {

    static_assert(sizeof...(Ts) > 0, "my_variant needs more than zero type arguments");
    static_assert(std::is_default_constructible<typename first_type<Ts...>::type>::value, "first type is needed to be deafult constuctable");

    template<typename T, typename... Os>
    friend struct my_variant_helper;

//    using my_variant_helper<int, Ts...>:_variant_helper;
//    using my_variant_helper<std::string, Ts...>::my_variant_helper;
//    using my_variant_helper<char, Ts...>::my_variant_helper;

    using vr::my_variant_helper<Ts, Ts...>::my_variant_helper...;

    /// constructors ///
    my_variant() :
        my_variant_helper<typename first_type<Ts...>::type, Ts...>(typename first_type<Ts...>::type()) {}

    my_variant(my_variant const& other) {
        other.visit([&](auto const& value) {
            *this = value;
        });
//        (my_variant_helper<typename std::remove_cv<typename std::remove_reference<decltype(other.get<Ts>())>::type>::type, Ts...>::operator =(other),...);
    }

    my_variant(my_variant&& other) {
        other.visit([&](auto const& value) {
            *this = value;
        });
//        (my_variant_helper<typename std::remove_cv<typename std::remove_reference<decltype(other.get<Ts>())>::type>::type, Ts...>::operator =(std::move(other)),...);
    }

    template<typename... Rs>
    my_variant(my_variant<Rs...> const& other) {
        other.visit([&](auto const& value) {
//            cerr << typeid(decltype(*this)).name() << endl;
//            cerr << typeid(decltype(other)).name() << endl;
            *this = value;
        });
    }

    template<typename... Rs>
    my_variant(my_variant<Rs...>&& other);

//    template<typename T, typename = typename std::enable_if<!details::utility::is_my_variant<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value>::type>
//    my_variant(T&& other)
//        : this->template my_variant_helper<T, Ts...>(std::forward<T>(other)) {}


    /// explore ///
    template<typename T> bool holds_alternative() const {
//        using this->my_variant_helper<T, Ts...>::id;
        return this->get_index() == vr::my_variant_helper<T, Ts...>::id;
    }

    template<typename T> T& get() {
        assert(holds_alternative<T>());
        return *this->template get_storage_pointer<T>();
    }

    template<typename T> T const& get() const {
        assert(holds_alternative<T>());
        return *this->template get_storage_pointer<T>();
    }

    size_t index() const noexcept {
        return this->get_index();
    }

    /// visits ///
    template<typename Visitor, typename Result = details::visit_details::__empty_result_type>
    visit_result<Result, Visitor, Ts...> visit(Visitor&& visitor)	{
            using Result_type = visit_result<Result, Visitor, Ts...>;
            return visit_impl<Result_type>(*this, std::forward<Visitor>(visitor), std::tuple<Ts...>());
    }

    template<typename Visitor, typename Result = details::visit_details::__empty_result_type>
    visit_result<Result, Visitor, Ts const...> visit(Visitor&& visitor) const {
            using Result_type = visit_result<Result, Visitor, Ts const...>;
            return vr::visit_impl<Result_type>(*this, std::forward<Visitor>(visitor), std::tuple<Ts...>());
    }

    /// assignment ///
    using vr::my_variant_helper<Ts, Ts...>::operator =...;

    my_variant& operator =(my_variant const& other) {
        other.visit([&](auto const& value) {
            *this = value;
        });
    }

    my_variant& operator =(my_variant&& other) {
        other.visit([&](const auto&& value) {
            *this = std::move(value);
        });
    }

    void destroy() {
//        cerr << "here" << endl;
        (vr::my_variant_helper<Ts, Ts...>::destroy(),...);
        this->set_index(sizeof...(Ts));
    }

    ~my_variant() {
        destroy();
    }

};

//             template<typename _Visitor, typename... _Variants>
//               constexpr decltype(auto)
//               visit(_Visitor&& __visitor, _Variants&&... __variants)
//               {
//                 if ((__variants.valueless_by_exception() || ...))
//               __throw_bad_variant_access("Unexpected index");

//                 using _Result_type =
//               decltype(std::forward<_Visitor>(__visitor)(
//                   get<0>(std::forward<_Variants>(__variants))...));

//                 constexpr auto& __vtable = __detail::__variant::__gen_vtable<
//               _Result_type, _Visitor&&, _Variants&&...>::_S_vtable;

//                 auto __func_ptr = __vtable._M_access(__variants.index()...);
//                 return (*__func_ptr)(std::forward<_Visitor>(__visitor),
//                          std::forward<_Variants>(__variants)...);
//               }

}


#endif // VARIANT_H
