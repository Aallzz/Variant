#ifndef VARIANT_H
#define VARIANT_H

#include <type_traits>

namespace details {
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


    void __variant_destroy(size_t, void*) {}

    template<typename T, typename... Ts>
    void __variant_destroy(size_t type_id, void* pdata) {
        if (type_id == typeid(T).hash_code())
            reinterpret_cast<T*>(pdata)->~T();
        else
            __variant_destroy<Ts...>(type_id, pdata);
    }
}

template<typename... Ts>
struct variant {


    static_assert(!(std::is_void_v<Ts> || ...), "variant must have no void alternative");

    variant() {}

    variant(variant const& other)
        : _type_id(other._type_od), _data(other._data) {}

    variant(variant&& other)
        : _type_id(other._type_id), _data(other.data) {
        other.type_id = typeid(void).hash_code();
        other.destroy();
    }

    bool is_valueless() {
        return _type_id == typeid(void).hash_code();
    }

    variant& operator = (variant const& rhs) {
        if (is_valueless() && rhs.is_valueless()) return *this;
        if (rhs.is_valueless()) {
            destroy();
            return *this;
        }
        if (_type_id == rhs._type_id) {
            _data = rhs._data;
            return *this;
        }
        variant temp(rhs);
        std::swap(_type_id, temp._type_id);
        std::swap(_data, temp._data);
    }

    template<typename T, typename = std::enable_if<details::index_of_type<T, Ts...>::value != sizeof...(Ts), void>>
    variant& operator = (T&& rhs) {
        if (is_valueless()) {
            _type_id = typeid(T).hash_code();
        }
    }

    static const size_t _size = details::max_types_size<Ts...>::value;
    static const size_t _align = details::max_types_align<Ts...>::value;

    using type = typename std::aligned_storage<_size, _align>::type;

    std::size_t _type_id {typeid(void).hash_code()};
    type _data;

    void destroy() {
        if (is_valueless()) return ;
        details::__variant_destroy(_type_id, &_data);
        type_id = typeid(void).hash_code();
    }

};
#endif // VARIANT_H
