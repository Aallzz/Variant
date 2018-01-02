#ifndef TEMP_VARIANT_H
#define TEMP_VARIANT_H

#include <type_traits>
#include <iostream>
#include <new>
#include <utility>
#include <string>

using std::cerr;
using std::endl;


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

template<typename... T>
using max_types_size = __variant_max<sizeof(T)...>;

template<typename... T>
using max_types_align = __variant_max<alignof(T)...>;

template<typename... Ts>
struct index_of_type {
    static constexpr size_t value = 0;
};

template<typename T, typename F, typename... Rest>
struct index_of_type<T, F, Rest...> {
    static constexpr size_t value = std::integral_constant<size_t, std::is_same<T, F>::value ? 0 : index_of_type<T, Rest...>::value + 1> {};
};

template<typename... Ts>
struct __first_type;

template<typename T, typename... Ts>
struct __first_type<T, Ts...> {
    using type = T;
};


template<typename... Ts> struct my_variant;

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

    my_variant_helper(T&& other) {
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

    static constexpr size_t id = index_of_type<T, Ts...>::value;

private:

    Derived& get_derived() {return *static_cast<Derived*>(this); }
    Derived const& get_derived() const {return *static_cast<Derived const*>(this); }

};

template<typename... Ts>
struct my_variant : private my_variant_storage<Ts...>, private my_variant_helper<Ts, Ts...>... {

    static_assert(sizeof...(Ts) > 0, "my_variant needs more than zero type arguments");
    static_assert(std::is_default_constructible<typename __first_type<Ts...>::type>::value, "first type is needed to be deafult constuctable");

    template<typename T, typename... Os>
    friend struct my_variant_helper;

//    using my_variant_helper<int, Ts...>::my_variant_helper;
//    using my_variant_helper<std::string, Ts...>::my_variant_helper;
//    using my_variant_helper<char, Ts...>::my_variant_helper;

    /// constructors ///
    my_variant() :
        my_variant_helper<typename __first_type<Ts...>::type, Ts...>(typename __first_type<Ts...>::type()) {}

    my_variant(my_variant const& other) {
        *this = other
    }
    my_variant(my_variant&& other);

    template<typename... Rs>
    my_variant(my_variant<Rs...> const& other);

    template<typename... Rs>
    my_variant(my_variant<Rs...>&& other);

    template<typename T>
    my_variant(T&& other)
        : my_variant_helper<T, Ts...>(std::forward<T>(other)) {}

    /// assignment ///
    using my_variant_helper<Ts, Ts...>::operator =...;

    /// explore ///
    template<typename T> bool is() const {
        return this->get_index() == my_variant_helper<T, Ts...>::id;
    }

    template<typename T> T& get() & {
        assert(is<T>());
        return *this->template get_storage_pointer<T>();
    }

    template<typename T> T&& get() &&;
    template<typename T> T const& get() const&;

    void destroy() {
//        cerr << "here" << endl;
        (my_variant_helper<Ts, Ts...>::destroy(),...);
        this->set_index(sizeof...(Ts));
    }

    ~my_variant() {
        destroy();
    }

private:

};

#endif // TEMP_VARIANT_H
