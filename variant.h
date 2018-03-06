#ifndef VARIANT_H
#define VARIANT_H

#include <type_traits>
#include <iostream>
#include <new>
#include <utility>
#include <string>
#include <array>

namespace vr {

template<typename... Ts>
struct variant;

/////////////////////////////////////////////////////// Helper objects //////////////////////////////////////////////////////////////////////////

inline constexpr std::size_t variant_npos = -1;

/////////////////////////////////////////////////////// END Helper objects //////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////// Helper classes //////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct variant_size;

template<typename... Ts>
struct variant_size<vr::variant<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename T> struct variant_size<const T> : variant_size<T> {};
template <typename T> struct variant_size<volatile T> : variant_size<T> {};
template <typename T> struct variant_size<const volatile T> : variant_size<T> {};

template <class T>
inline constexpr std::size_t variant_size_v = variant_size<T>::value;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <std::size_t I, typename T>
struct variant_alternative;

template <std::size_t I, typename... Types>
struct variant_alternative<I, variant<Types...>>;

template<std::size_t I, typename T, typename... Types>
struct variant_alternative<I, variant<T, Types...>> {
    using type = typename variant_alternative<I - 1, variant<Types...>>::type;
};

template<typename T, typename... Types>
struct variant_alternative<0, variant<T, Types...>> {
    using type = T;
};

template <size_t I, typename T> struct variant_alternative<I, const T> : variant_alternative<I, T> {};
template <size_t I, typename T> struct variant_alternative<I, volatile T> : variant_alternative<I, T> {};
template <size_t I, typename T> struct variant_alternative<I, const volatile T> : variant_alternative<I, T> {};

template <size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
///     std::get(std::variant) called with an index or type that does not match the currently active alternative
///     std::visit called to visit a variant that is valueless_by_exception

class bad_variant_access : public std::exception {};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct monostate {};

constexpr bool operator<(monostate, monostate) noexcept { return false; }
constexpr bool operator>(monostate, monostate) noexcept { return false; }
constexpr bool operator<=(monostate, monostate) noexcept { return true; }
constexpr bool operator>=(monostate, monostate) noexcept { return true; }
constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr bool operator!=(monostate, monostate) noexcept { return false; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// http://en.cppreference.com/w/cpp/utility/hash
///

//template<typename... Types>
//struct hash<variant<Types...>> {};                                                                                    //////// TODO

template<typename T>
struct hash;

template <>
struct hash<monostate> {
    std::size_t operator()(monostate const&) const noexcept {
        return 11111111;
}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////// END Helper classes //////////////////////////////////////////////////////////////////////////



namespace details {

/////////////////////////////////////////////////////////////////////////////

    template<typename T, typename... Ts>
    struct find_type;

    template<typename T, typename F, typename... Ts>
    struct find_type<T, F, Ts...> {
        constexpr static bool value = std::is_same_v<std::decay_t<T>, std::decay_t<F>> || find_type<T, Ts...>::value;
    };

    template<typename T>
    struct find_type<T> {
        constexpr static bool value = false;
    };

    template<typename T, typename... Ts>
    inline constexpr bool find_type_v = find_type<T, Ts...>::value;

////////////////////////////////////////////////////////////////////////////

    template<int id, typename... Ts>
    struct get_type_at;

    template<int id, typename F, typename... Ts>
    struct get_type_at<id, F, Ts...> {
        using type = typename get_type_at<id - 1, Ts...>::type;
    };

    template<typename F, typename... Ts>
    struct get_type_at<0, F, Ts...> {
        using type = F;
    };

    template<int id, typename... Ts>
    using get_type_at_t = get_type_at<id, Ts...>;

////////////////////////////////////////////////////////////////////////////

    template<typename T, typename... Ts>
    struct get_id_at;

    template<typename T, typename F, typename... Ts>
    struct get_id_at<T, F, Ts...> {
        constexpr static std::size_t value = std::is_same_v<T, F> ? 0 : (get_id_at<T, Ts...>::value + 1);
    };

    template<typename T>
    struct get_id_at<T> {
        constexpr static std::size_t value = 0;
    };

    template<typename T, typename... Ts>
    struct not_unique_type;

    template<typename T, typename F, typename... Ts>
    struct not_unique_type<T, F, Ts...> {
        constexpr static bool value = std::is_same_v<T, F> || not_unique_type<T, Ts...>::value;
    };

    template<typename T>
    struct not_unique_type<T> {
        constexpr static bool value = 0;
    };

    template<typename T, typename... Ts>
    inline constexpr bool is_unique_type_v = !not_unique_type<T, Ts...>::value;

    template<typename T, typename... Ts>
    struct get_id_at_checked {
        static_assert(is_unique_type_v<T, Ts...>, "PROBLEM 1");
        constexpr static std::size_t value = get_id_at<T, Ts...>::value;
        static_assert(value < sizeof...(Ts), "PROBLEM 2");
    };

    template<typename T, typename... Ts>
    inline constexpr std::size_t get_id_at_v = get_id_at_checked<T, Ts...>::value;

////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////

    namespace storage {
        template<typename... Ts>
        struct storage_base;

    }


////////////////////////////////////////////////////////////////////////////

    namespace visitor {

        template<typename T>
        constexpr static const T& at(T const& value) {
            return value;
        }

        template<std::size_t N, typename T, typename... RestCoordinates>
        constexpr static const T& at(std::array<T, N> const& ar, std::size_t x, RestCoordinates... rest_coordinates) {
            return at(ar[x], rest_coordinates...);
        }

        template <typename F, typename... Fs>
        static constexpr int visit_visitor_return_type_check() {
            static_assert(is_unique_type_v<F, Fs...>, "`visit` requires the visitor to have a single ""return type.");
            return 0;
        }

        template <std::size_t... Is>
        struct visit_agency {
          template <typename F, typename... Vs>
          struct visitor {
              /// Вызывает f с активными элементами вариантов
              /// Is... - индексы "активных" элементов
              constexpr static decltype(auto) visit(F f, Vs... vs) {
                  return std::invoke(static_cast<F>(f), storage::storage_base<Vs...>::get_alternative<Is>(static_cast<Vs>(vs))...);
              }
          };
        };

        template<typename... Fs, typename res_t = std::array<std::common_type_t<std::decay_t<Fs>...>, sizeof...(Fs)>>
        constexpr static res_t make_farray(Fs&&... fs) {
            return res_t({std::forward<Fs>(fs)...});
        }

        template<typename F, typename... Vs, std::size_t... Is>
        constexpr static decltype(auto) make_visit(std::index_sequence<Is...>) {
            return (visit_agency<Is...>::template visitor<F, Vs...>::visit);
        }

        /// просто костыль
        template<std::size_t id, typename T>
        struct wrap_pair {
            constexpr static std::size_t value = id;
        };

        template<std::size_t I, typename F, typename... Vs>
        constexpr static auto make_fdiagonal_impl() {
            return make_visit<F, Vs...>(std::index_sequence<wrap_pair<I, Vs>::value...>());
        }

        template<typename F, typename... Vs, std::size_t... Is>
        constexpr static auto make_fdiagonal_impl(std::index_sequence<Is...>) {
            return make_farray(make_fdiagonal_impl<Is, F, Vs...>()...);
        }

        template<typename F, typename V, typename... Vs>
        constexpr static decltype(auto) make_fdiagonal() {
            static_assert((std::is_same_v<std::decay_t<V>::size(), std::decay_t<Vs>::size()> && ...), "all of the variants must be the same size.");
            return make_fdiagonal_impl<F, V, Vs...>(std::make_index_sequence<std::decay_t<V>::size()>{});
        }

        template <typename F, typename... Vs>
        struct fdiagonal {
            using type = decltype(make_fdiagonal<F, Vs...>());
            static constexpr type value = make_fdiagonal<F, Vs...>();
        };



        template<typename F, typename... Vs, typename Is>
        constexpr static decltype(auto) make_f_matrix_impl(Is is) {
            return make_visit<F, Vs...>(is);
        }

        template<typename F, typename... Vs, std::size_t... Is, std::size_t... IAs, typename... RestIS>
        constexpr static decltype(auto) make_f_matrix_impl(std::index_sequence<Is...>, std::index_sequence<IAs...>, RestIS... rest) {
            return make_farray(make_f_matrix_impl<F, Vs...>(std::index_sequence<Is..., IAs>{}, rest...)...);
        }


        template<typename F, typename... Vs>
        constexpr static decltype(auto) make_fmatrix() {
            return make_f_matrix_impl<F, Vs...>(std::index_sequence<>(), std::index_sequence<std::decay_t<Vs>::size()>{}...);
        }

        /// Создает N-мерную "кубичискую" матрицу, длина "стороны" которой varaint::size()
        template <typename F, typename... Vs>
        struct fmatrix {
            using type = decltype(make_fmatrix<F, Vs...>());
            static constexpr type value = make_fmatrix<F, Vs...>();
        };

        ////////////////////////////////////////////////////////////////////
        /// Для себя, если забуду, что тут написано:
        /// Если мы хотим применить функцию Visitor к Variant'ам (Vs...), то для этого
        /// 1) Строим матрицу, в ячейках которых находится функция visit, которая вызовет std::invoke
        ///    коориданты этой функции - индексы из вариантов, которые предполагаются активированными
        /// 2) В этой матрице обращаемся к элементу с координатами vs1.index(), vs2.index(), ....,
        ///    получая таким образому функцию, которую нам нужно вызвать
        /// 3) Вызываем функцию, передавая ей Visitor и Vs..., что будет равнсильно Visitor(Vs...) // тут форварды всякией
        ///
        /// Если мы хотим применить функцию Visitor к Variantam, активные индексы которых совпадают и мы его знаем (index) то
        /// 1) Строим что-то вроде главной диагонали с координатами 000, 111 и тп
        /// 2) Аналогично находим нужную ячейку, функцию которой вызовем
        ///
        template<typename Visitor, typename... Vs>
        constexpr static decltype(auto) visit_alt_at(std::size_t index, Visitor&& visitor, Vs&&... vs) {
            return at(fdiagonal<Visitor&&, decltype(std::forward<Vs>(vs))...>::value, index)(std::forward<Visitor>(visitor), std::forward<Vs>(vs)...);
        }

        template<typename Visitor, typename... Vs>
        constexpr static decltype(auto) visit_alt(Visitor&& visitor, Vs&&... vs) {
            return at(fmatrix<Visitor&&, decltype(std::forward<Vs>(vs))...>::value, vs.index()...)(std::forward<Visitor>(visitor), std::forward<Vs>(vs)...);
        }
                                                                                                                                    /// TODO CHECK IF INOKE CALL IS POSSIBLE
        template<typename Visitor, typename... Vs>                                                                                                                        \
        struct visit_with_check {
            //static_assect(chech_invoke, "impossible to invoke");
            constexpr decltype(auto) operator()(Visitor&& visitor, Vs&&... values) const {
                return std::invoke(std::forward<Visitor>(visitor), std::forward<Vs>(values)...);
            }
        };

        template<typename Visitor>
        struct value_visitor {
            std::remove_reference_t<Visitor> visitor;

            value_visitor(Visitor&& v) : visitor(std::forward<Visitor>(v)) {}

            template<typename... Alts>
            constexpr decltype(auto) operator()(Alts&&... alts) const {
                return visit_with_check<Visitor, decltype(std::forward<Alts>(alts).value)...>()(std::forward<Visitor>(visitor), std::forward<Alts>(alts).value...);
            }
        };

        template<typename Visitor>
        constexpr static decltype(auto) make_value_visitor(Visitor&& value) {
            return value_visitor<Visitor>(std::forward<Visitor>(value));
        }

        template<typename Visitor, typename... Vs>
        constexpr static decltype(auto) visit_variant_at(std::size_t index, Visitor&& visitor, Vs&&... vs) {
            return visit_alt_at(index, std::forward<Visitor>(visitor), std::forward<Vs>(vs).storage...);
        }

        template<typename Visitor, typename... Vs>
        constexpr static decltype(auto) visit_variant(Visitor&& visitor, Vs&&... vs) {
            return visit_alt(std::forward<Visitor>(visitor), std::forward<Vs>(vs)...);
        }

        template<typename Visitor, typename... Vs>
        constexpr static decltype(auto) visit_value_at(std::size_t index, Visitor&& visitor, Vs&&... vs) {
            return visit_variant_at(index, make_value_visitor(std::forward<Visitor>(visitor)), std::forward<Vs>(vs)...);
        }

        template<typename Visitor, typename... Vs>
        constexpr static decltype(auto) visit_value(Visitor&& visitor, Vs&&... vs) {
            return visit_alt(make_value_visitor(std::forward<Visitor>(visitor)), std::forward<Vs>(vs)...);
        }


    } // end visitor


//    The call is ill-formed if the invocation above is not a valid expression of the same type and
//    value category, for all combinations of alternative types of all variants.


////////////////////////////////////////////////////////////////////////////


    namespace storage {


        /////////////////////////////////////////////////////////////////////////////////

        template<typename std::size_t ind, typename T>
        struct alternative {

            template<typename... As>
            explicit constexpr alternative(std::in_place_t, As&&... args)
                : value(std::forward<As>(args)...)
            {}

            T value;
        };

        /////////////////////////////////////////////////////////////////////////////////

        template<std::size_t ind, typename... Ts>
        union union_storage;

        template<std::size_t ind>
        union union_storage<ind>
        {};

        template<std::size_t ind, typename T, typename... Ts>
        union union_storage<ind, T, Ts...>   {

            template<typename... As>
            explicit constexpr union_storage(std::in_place_index_t<0>, As&&... args)
                : head(std::in_place_t(), std::forward<As>(args)...)
            {}

            template<std::size_t id, typename... As>
            explicit constexpr union_storage(std::in_place_index_t<id>, As&&... args)
                : tail(std::in_place_index_t<id - 1>(), std::forward<As>(args)...)
            {}

            template <typename V>
            static constexpr auto&& get_alternative(V &&v, std::in_place_index_t<0>) {
                return std::forward<V>(v).head;
            }

            template <std::size_t id, typename V>
            static constexpr auto&& get_alternative(V &&v, std::in_place_index_t<id>) {
                return get_alternative(std::forward<V>(v).tail, std::in_place_index_t<id - 1>{});
            }

            ~union_storage() = default;

        private:

            alternative<ind, T> head;
            union_storage<ind + 1, Ts...> tail;
        };

        /////////////////////////////////////////////////////////////////////////////////

        template<typename... Ts>
        struct storage_base {

            template<std::size_t ind, typename... As>
            storage_base(std::in_place_index_t<ind>, As&&... args)
                : data(std::in_place_index_t<ind>(), std::forward<As>(args)...), indx(ind)
            {
                std::cerr << "in_storage_base_impl with ind = " << ind << "\n";
            }

            constexpr bool valueless_by_exception() const noexcept {
                    return indx == static_cast<std::size_t>(-1);
            }

            constexpr std::size_t index() const noexcept {
                    return valueless_by_exception() ? variant_npos : indx;
            }

            template <std::size_t id, typename Variant>
            static constexpr auto&& get_alternative(Variant&& v) {
                return union_storage<0, Ts...>::get_alternative(union_storage<0, Ts...>(std::forward<Variant>(v)), std::in_place_index_t<id>{});
            }

            template <typename T>
            static constexpr const T &at(T const& a) {
                return a;
            }

        protected:

            union_storage<0, Ts...> data;
            std::size_t indx;
        };

        ////////////////////////////////////// Add destructors ///////////////////////////////////////////
        ///  If valueless_by_exception is true, does nothing. Otherwise, destroys the currently contained value.
        ///  This destructor is trivial if std::is_trivially_destructible_v<T_i> is true for all T_i in Types...

        template<bool d, typename... Ts>
        struct storage_destructor_part;

        template<typename... Ts>
        struct storage_destructor_part<true, Ts...> : storage_base<Ts...> {
            constexpr void destroy() {
                this->indx = variant_npos;
            }

            ~storage_destructor_part() {
                destroy();
            }
        };

        template<typename... Ts>
        struct storage_destructor_part<false, Ts...> : storage_base<Ts...> {
            constexpr void destroy() {
                if (!this->valueless_by_exception()) {
                    visitor::visit_alt([](auto& value){using type = std::remove_reference_t<decltype(value)>; value.~type();}), *this;
                }
                this->atindx = variant_npos;
            }

            ~storage_destructor_part() {
                destroy();
            }
        };

        ////////////////////////////////////// Add contructors ///////////////////////////////////////////

        template<bool d, typename... Ts>
        struct storage_constructor_part;

        template<typename... Ts>
        struct storage_constructor_part<true, Ts...> : storage_destructor_part<(std::is_trivially_destructible_v<Ts> && ...), Ts...> {

            struct ctor {
                    template <typename T, typename U>
                    inline void operator()(T& t, U&& u) const {
                      storage_constructor_part::construct_alternative(t, std::forward<U>(u).value);
                    }
            };

            template <std::size_t I, typename T, typename... Args>
            constexpr static T &construct_alternative(alternative<I, T> &a, Args &&... args) {
                new (static_cast<void*>(__builtin_addressof(a))) alternative<I, T>(std::in_place_t(), std::forward<Args>(args)...);
                    return a.value;
            }

            template <typename Rhs>
            static void generic_construct(storage_constructor_part &lhs, Rhs &&rhs) {
                lhs.destroy();
                if (!rhs.valueless_by_exception()) {
                      visitor::visit_alt_at(rhs.index(), ctor{}, lhs, std::forward<Rhs>(rhs));
                      lhs.index_ = rhs.index_;
                     }
                    }


            storage_constructor_part(storage_constructor_part const& other) = default;
        };

        template<typename... Ts>
        struct storage_constructor_part<false, Ts...> : storage_destructor_part<(std::is_trivially_destructible_v<Ts> && ...), Ts...> {
            storage_constructor_part(storage_constructor_part const& other) {
                      this->generic_construct(*this, other);
            };
        };

        ////////////////////////////////////// Add copy contructors ///////////////////////////////////////////

        template<bool d, typename... Ts>
        struct storage_cconstructor_part;

        template<typename... Ts>
        struct storage_cconstructor_part<true, Ts...> : storage_constructor_part<(std::is_trivially_constructible_v<Ts> && ...), Ts...> {
        };

        template<typename... Ts>
        struct storage_cconstructor_part<false, Ts...> : storage_constructor_part<(std::is_trivially_constructible_v<Ts> && ...), Ts...> {
        };

        ////////////////////////////////////// Add move contructor ///////////////////////////////////////////

        template<bool d, typename... Ts>
        struct storage_mconstructor_part;

        template<typename... Ts>
        struct storage_mconstructor_part<true, Ts...> : storage_cconstructor_part<(std::is_trivially_copy_constructible_v<Ts> && ...), Ts...> {

        };

        template<typename... Ts>
        struct storage_mconstructor_part<false, Ts...> : storage_cconstructor_part<(std::is_trivially_copy_constructible_v<Ts> && ...), Ts...> {

        };

        /////////////////////////////////////////////////////////////////////////////////




        template<typename... Ts>
        struct storage_t {

//            template<typename... As>
//            storage_t(As&&... args)
//                : "base"(std::forward<As>(args)...);
//            {};
        };
} // storage end

} // details end

/////////////////////////////////////////////////////// Non-member functions //////////////////////////////////////////////////////////////////////////

//template <class T, class... Types>
//constexpr bool holds_alternative(const std::variant<Types...>& v) noexcept {
//
//}

/////////////////////////////////////////////////////// END Non-member functions //////////////////////////////////////////////////////////////////////////



template<typename... Ts>
struct variant {

    static_assert(0 < sizeof...(Ts), "variant must have at least one alternative");
    static_assert(details::find_type_v<void, Ts...>, "variant must have at least one alternative");

    template<std::enable_if_t<std::is_default_constructible_v<details::get_type_at_t<0, Ts...>>>* = nullptr>
    constexpr variant() noexcept(std::is_nothrow_default_constructible_v<details::get_type_at_t<0, Ts...>>)
        : storage(std::in_place_index_t<0>{})
    {}

    constexpr std::size_t index() const noexcept {

    }

    constexpr bool valueless_by_exception() const noexcept {

    }

    constexpr std::size_t size() const {
        return sizeof...(Ts);
    }

private:

    details::storage::storage_t<Ts...> storage;

};

} // end vr


#endif // VARIANT_H
