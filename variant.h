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
///     std::get(variant) called with an index or type that does not match the currently active alternative
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

    template<std::size_t id, typename... Ts>
    struct get_type_at;

    template<std::size_t id, typename F, typename... Ts>
    struct get_type_at<id, F, Ts...> {
        using type = typename get_type_at<id - 1, Ts...>::type;
    };

    template<typename F, typename... Ts>
    struct get_type_at<0, F, Ts...> {
        using type = F;
    };

    template<std::size_t id, typename... Ts>
    using get_type_at_t = typename get_type_at<id, Ts...>::type;

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
        constexpr static std::size_t value = std::is_same_v<T, F> + not_unique_type<T, Ts...>::value;
    };

    template<typename T>
    struct not_unique_type<T> {
        constexpr static bool value = 0;
    };

    template<typename T, typename... Ts>
    inline constexpr bool is_unique_type_v = not_unique_type<T, Ts...>::value <= 1;

    template<typename T, typename... Ts>
    struct get_id_at_checked {
        static_assert(is_unique_type_v<T, Ts...>, "PROBLEM 1");
        constexpr static std::size_t value = get_id_at<T, Ts...>::value;
    };

    template<typename T, typename... Ts>
    inline constexpr std::size_t get_id_at_v = get_id_at_checked<T, Ts...>::value;

////////////////////////////////////////////////////////////////////////////

    template<typename T>
    struct is_in_place {
        constexpr static bool value = false;
    };

    template<>
    struct is_in_place<decltype(std::in_place)> {
        constexpr static bool value = false;
    };

    template<typename T>
    constexpr bool is_in_place_v = is_in_place<T>::value;

////////////////////////////////////////////////////////////////////////////

    template<typename T>
    struct is_in_place_index {
        constexpr static bool value = false;
    };

    template<std::size_t I>
    struct is_in_place_index<std::in_place_index_t<I>> {
        constexpr static bool value = true;
    };

    template<typename T>
    constexpr bool is_in_place_index_v = is_in_place_index<T>::value;

////////////////////////////////////////////////////////////////////////////

    template<typename T>
    struct is_in_place_type {
        constexpr static bool value = false;
    };

    template<typename T>
    struct is_in_place_type<std::in_place_type_t<T>> {
        constexpr static bool value = true;
    };

    template<typename T>
    constexpr static bool is_in_place_type_v = is_in_place_type<T>::value;

////////////////////////////////////////////////////////////////////////////

    template<typename Is, typename... Ts>
    struct fake_functions;

    template<std::size_t I, std::size_t... Is, typename T, typename... Ts>
    struct fake_functions<std::index_sequence<I, Is...>, T, Ts...> : fake_functions<std::index_sequence<Is...>, Ts...> {

        using fake_functions<std::index_sequence<Is...>, Ts...>::operator();

        std::integral_constant<std::size_t, I> operator ()(T);
    };

    template<std::size_t I, typename T>
    struct fake_functions<std::index_sequence<I>, T> {
        std::integral_constant<std::size_t, I> operator ()(T);
    };

    template<typename T, typename... Ts, std::enable_if_t<std::is_invocable_v<fake_functions<std::index_sequence_for<Ts...>, Ts...>, T>>* = nullptr>
    constexpr std::size_t find_best_match_v = std::invoke_result_t<fake_functions<std::index_sequence_for<Ts...>, Ts...>, T&&>::value;

////////////////////////////////////////////////////////////////////////////

    namespace storage {
        template<typename... Ts>
        struct storage_base;

    }


////////////////////////////////////////////////////////////////////////////

    namespace access {

        struct union_storage_helper {
            template <typename V>
            static constexpr auto&& get_alternative(V&& v, std::in_place_index_t<0>) {
              return std::forward<V>(v).head;
            }

            template <typename V, std::size_t I>
            static constexpr auto&& get_alternative(V&& v, std::in_place_index_t<I>) {
              return get_alternative(std::forward<V>(v).tail, std::in_place_index_t<I - 1>());
            }
        };

        struct storage_base_helper {
            template <std::size_t I, typename V>
            static constexpr auto&& get_alternative(V&& v) {
              return union_storage_helper::get_alternative(std::forward<V>(v).data, std::in_place_index_t<I>{});
            }
        };

        struct variant_helper {
            template <std::size_t I, typename V>
            static constexpr auto&& get_alternative(V&& v) {
              return storage_base_helper::get_alternative<I>(std::forward<V>(v).storage);
            }
        };

    }

    namespace visitor {

        struct storage_base_helper {

            template<typename T>
            constexpr static const T& at(T const& value) {
                return value;
            }

            template<std::size_t N, typename T, typename... RestCoordinates>
            constexpr static auto const& at(std::array<T, N> const& ar, std::size_t x, RestCoordinates... rest_coordinates) {
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
                      return std::invoke(static_cast<F>(f), access::storage_base_helper::get_alternative<Is>(static_cast<Vs>(vs))...);
                  }
              };
            };

            template<typename... Fs, typename res_t = std::array<std::common_type_t<std::decay_t<Fs>...>, sizeof...(Fs)>>
            constexpr static res_t make_farray(Fs&&... fs) {
                return res_t{{std::forward<Fs>(fs)...}};
            }

            template<typename F, typename... Vs, std::size_t... Is>
            constexpr static auto make_visit(std::index_sequence<Is...>) {
                return visit_agency<Is...>::template visitor<F, Vs...>::visit;
            }

            template<int N>
            struct print_size_as_warning
            {
               char operator()() { return N + 256; } //deliberately causing overflow
            };

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
                static_assert((std::is_same_v<std::integral_constant<std::size_t, std::decay_t<V>::size()>, std::integral_constant<std::size_t, std::decay_t<Vs>::size()>> && ...), "all of the variants must be the same size.");
                return make_fdiagonal_impl<F, V, Vs...>(std::make_index_sequence<std::decay_t<V>::size()>{});
            }

            template<typename F, typename... Vs, typename Is>
            constexpr static auto make_f_matrix_impl(Is is) {
                return make_visit<F, Vs...>(is);
            }

            template<typename F, typename... Vs, std::size_t... Is, std::size_t... IAs, typename... RestIS>
            constexpr static auto make_f_matrix_impl(std::index_sequence<Is...>, std::index_sequence<IAs...>, RestIS... rest) {
//                print_size_as_warning<sizeof...(RestIS)>()();
                return make_farray(make_f_matrix_impl<F, Vs...>(std::index_sequence<Is..., IAs>{}, rest...)...);
            }


            template<typename F, typename... Vs>
            constexpr static decltype(auto) make_fmatrix() {
                return make_f_matrix_impl<F, Vs...>(std::index_sequence<>(), std::make_index_sequence<std::decay_t<Vs>::size()>{}...);
            }

        };

        template <typename F, typename... Vs>
        struct fdiagonal {
            using type = decltype(storage_base_helper::make_fdiagonal<F, Vs...>());
            static constexpr type value = storage_base_helper::make_fdiagonal<F, Vs...>();
        };

        /// Создает N-мерную "кубичискую" матрицу, длина "стороны" которой varaint::size()
        template <typename F, typename... Vs>
        struct fmatrix {
            using type = decltype(storage_base_helper::make_fmatrix<F, Vs...>());
            static constexpr type value = storage_base_helper::make_fmatrix<F, Vs...>();
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
            return storage_base_helper::at(fdiagonal<Visitor&&, decltype(std::forward<Vs>(vs))...>::value, index)
                                         (std::forward<Visitor>(visitor), std::forward<Vs>(vs)...);
        }

        template<typename Visitor, typename... Vs>
        constexpr static decltype(auto) visit_alt(Visitor&& visitor, Vs&&... vs) {
            auto x = storage_base_helper::at(fmatrix<Visitor&&, decltype(std::forward<Vs>(vs))...>::value, vs.index()...);
            return x(std::forward<Visitor>(visitor), std::forward<Vs>(vs)...);
        }


        struct variant_helper {

            template<typename Visitor, typename... Vs>                                                                                                                        \
            struct visit_with_check {
                //static_assect(chech_invoke, "impossible to invoke");
                constexpr decltype(auto) operator()(Visitor&& visitor, Vs&&... values) const {
                    return std::invoke(std::forward<Visitor>(visitor), std::forward<Vs>(values)...);
                }
            };


            template<typename Visitor>
            struct value_visitor {
                Visitor&& visitor;

                constexpr value_visitor(Visitor&& v) : visitor(std::forward<Visitor>(v)) {}

                template<typename... Alts>
                constexpr decltype(auto) operator()(Alts&&... alts) const {
                    return visit_with_check<Visitor,
                                            decltype((std::forward<Alts>(alts).value))...
                            >()(std::forward<Visitor>(visitor), std::forward<Alts>(alts).value...);
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
                return visit_alt(std::forward<Visitor>(visitor), std::forward<Vs>(vs).storage...);
            }

            template<typename Visitor, typename... Vs>
            constexpr static decltype(auto) visit_value_at(std::size_t index, Visitor&& visitor, Vs&&... vs) {
                return visit_variant_at(index, make_value_visitor(std::forward<Visitor>(visitor)), std::forward<Vs>(vs)...);
            }

            template<typename Visitor, typename... Vs>
            constexpr static decltype(auto) visit_value(Visitor&& visitor, Vs&&... vs) {
                return visit_variant(make_value_visitor(std::forward<Visitor>(visitor)), std::forward<Vs>(vs)...);
            }
        };

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

            alternative() = default;
            alternative(alternative const&) = default;
            alternative(alternative&&) = default;
            alternative& operator =(alternative const&) = default;
            alternative& operator =(alternative&&) = default;
            ~alternative() = default;

            T value;
        };

        /////////////////////////////////////////////////////////////////////////////////

        template<bool b, std::size_t ind, typename... Ts>
        union union_storage;

        template<bool b, std::size_t ind>
        union union_storage<b, ind>
        {};

        template<std::size_t ind, typename T, typename... Ts>
        union union_storage<true, ind, T, Ts...>   {

            union_storage() {}

            template<typename... As>
            explicit constexpr union_storage(std::in_place_index_t<0>, As&&... args)
                : head(std::in_place, std::forward<As>(args)...)
            {}

            template<std::size_t id, typename... As>
            explicit constexpr union_storage(std::in_place_index_t<id>, As&&... args)
                : tail(std::in_place_index<id - 1>, std::forward<As>(args)...)
            {}

            friend struct access::union_storage_helper;

            union_storage(union_storage const&) = default;
            union_storage(union_storage&&) = default;

            union_storage &operator=(const union_storage &) = default;
            union_storage &operator=(union_storage &&) = default;

            ~union_storage() = default;

        private:

            alternative<ind, T> head;
            union_storage<true, ind + 1, Ts...> tail;
        };

        template<std::size_t ind, typename T, typename... Ts>
        union union_storage<false, ind, T, Ts...>   {

            union_storage() {}

            template<typename... As>
            explicit constexpr union_storage(std::in_place_index_t<0>, As&&... args)
                : head(std::in_place_t(), std::forward<As>(args)...)
            {}

            template<std::size_t id, typename... As>
            explicit constexpr union_storage(std::in_place_index_t<id>, As&&... args)
                : tail(std::in_place_index_t<id - 1>(), std::forward<As>(args)...)
            {}

            friend struct access::union_storage_helper;

            union_storage(union_storage const&) = default;
            union_storage(union_storage&&) = default;

            union_storage& operator =(union_storage const &) {}
            union_storage& operator =(union_storage &&) = default;

            ~union_storage(){};

        private:

            alternative<ind, T> head;
            union_storage<false, ind + 1, Ts...> tail;
        };

        /////////////////////////////////////////////////////////////////////////////////

        template<typename... Ts>
        struct storage_base {

            storage_base() : data(), indx(-1) {}
            storage_base(storage_base const&) = default;
            storage_base(storage_base&&) = default;

            storage_base& operator =(storage_base const&) = default;
            storage_base& operator =(storage_base&&) = default;

            template<std::size_t ind, typename... As>
            constexpr storage_base(std::in_place_index_t<ind>, As&&... args)
                : data(std::in_place_index_t<ind>(), std::forward<As>(args)...), indx(ind)
            {
//                std::cerr << "in_storage_base_impl with ind = " << ind << "\n";
            }

            constexpr bool valueless_by_exception() const noexcept {
                    return indx == static_cast<std::size_t>(-1);
            }

            constexpr std::size_t index() const noexcept {
                    return valueless_by_exception() ? variant_npos : indx;
            }

            constexpr static std::size_t size() { return sizeof...(Ts); }

            ///////////////////https://stackoverflow.com/questions/29391422/declare-static-functions-as-friend-function
//        protected:

            using data_type = union_storage<(std::is_trivially_destructible_v<Ts> && ...),0, Ts...>;

            friend struct visitor::storage_base_helper;
            friend struct access::storage_base_helper;

            friend constexpr data_type &data(storage_base &b) { return b.data; }
            friend constexpr const data_type &data(const storage_base &b) { return b.data; }
            friend constexpr data_type &&data(storage_base &&b) { return std::move(b).data; }
            friend constexpr const data_type &&data(const storage_base &&b) { return std::move(b).data; }

            template <typename T>
            static constexpr const T &at(T const& a) {
                return a;
            }

            data_type data;
            std::size_t indx;
        };

        ////////////////////////////////////// Add destructors ///////////////////////////////////////////
        ///  If valueless_by_exception is true, does nothing. Otherwise, destroys the currently contained value.
        ///  This destructor is trivial if std::is_trivially_destructible_v<T_i> is true for all T_i in Types...

        template<bool d, typename... Ts>
        struct storage_destructor_part;

        template<typename... Ts>
        struct storage_destructor_part<true, Ts...> : storage_base<Ts...> {

            using storage_base<Ts...>::storage_base;

            storage_destructor_part(storage_destructor_part const&) = default;
            storage_destructor_part(storage_destructor_part&&) = default;

            storage_destructor_part& operator =(storage_destructor_part const&) = default;
            storage_destructor_part& operator =(storage_destructor_part&&) = default;

            constexpr void destroy() {
                this->indx = variant_npos;
            }

            ~storage_destructor_part() = default;
        };

        template<typename... Ts>
        struct storage_destructor_part<false, Ts...> : storage_base<Ts...> {

            using storage_base<Ts...>::storage_base;

            storage_destructor_part(storage_destructor_part const&) = default;
            storage_destructor_part(storage_destructor_part&&) = default;

            storage_destructor_part& operator =(storage_destructor_part const&) = default;
            storage_destructor_part& operator =(storage_destructor_part&&) = default;

            constexpr void destroy() {
                if (!this->valueless_by_exception()) {
                    visitor::visit_alt([](auto& value){using type = std::remove_reference_t<decltype(value)>; value.~type();}, *this);
                }
                this->indx = variant_npos;
            }

            ~storage_destructor_part() {
                destroy();
            }
        };

        ////////////////////////////////////// Add helpers for constructors ///////////////////////////////////////////

        template<typename... Ts>
        struct storage_constructor_part : storage_destructor_part<(std::is_trivially_destructible_v<Ts> && ...), Ts...> {

            using storage_destructor_part<(std::is_trivially_destructible_v<Ts> && ...), Ts...>::storage_destructor_part;

            storage_constructor_part(storage_constructor_part const&) = default;
            storage_constructor_part(storage_constructor_part&&) = default;

            storage_constructor_part& operator =(storage_constructor_part const&) = default;
            storage_constructor_part& operator =(storage_constructor_part&&) = default;

            template <std::size_t I, typename T, typename... Args>
            constexpr static T& construct_alternative(alternative<I, T>& a, Args&&... args) {
                new (&a) alternative<I, T>(std::in_place_t(), std::forward<Args>(args)...);
                return a.value;
            }

            template <typename T>
            static void construct(storage_constructor_part& where, T&& what) {
                where.destroy();
                if (!what.valueless_by_exception()) {
                    visitor::visit_alt_at(what.index(),
                                          [](auto& where_alt, auto&& what_alt) {
                                              storage_constructor_part::construct_alternative(where_alt, std::forward<decltype(what_alt)>(what_alt).value);
                                          },
                                          where, std::forward<T>(what));
                    where.indx = what.indx;
                }
            }
        };

        ////////////////////////////////////// Add copy contructors ///////////////////////////////////////////

        template<bool d, typename... Ts>
        struct storage_cconstructor_part;

        template<typename... Ts>
        struct storage_cconstructor_part<true, Ts...> : storage_constructor_part<Ts...> {

            using storage_constructor_part<Ts...>::storage_constructor_part;

            storage_cconstructor_part(storage_cconstructor_part const&) = default;
            storage_cconstructor_part(storage_cconstructor_part&&) = default;

            storage_cconstructor_part& operator =(storage_cconstructor_part const&) = default;
            storage_cconstructor_part& operator =(storage_cconstructor_part&&) = default;
        };

        template<typename... Ts>
        struct storage_cconstructor_part<false, Ts...> : storage_constructor_part<Ts...> {

            using storage_constructor_part<Ts...>::storage_constructor_part;

            storage_cconstructor_part(storage_cconstructor_part const& other)
                noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...))
                : storage_constructor_part<Ts...>()
            {
                this->construct(*this, other);
            }


            storage_cconstructor_part(storage_cconstructor_part&&) = default;

            storage_cconstructor_part& operator =(storage_cconstructor_part const&) = default;
            storage_cconstructor_part& operator =(storage_cconstructor_part&&) = default;

        };

        ////////////////////////////////////// Add move contructor ///////////////////////////////////////////

        template<bool d, typename... Ts>
        struct storage_mconstructor_part;

        template<typename... Ts>
        struct storage_mconstructor_part<true, Ts...> : storage_cconstructor_part<(std::is_trivially_copy_constructible_v<Ts> && ...), Ts...> {

            using storage_cconstructor_part<(std::is_trivially_copy_constructible_v<Ts> && ...), Ts...>::storage_cconstructor_part;

            storage_mconstructor_part(storage_mconstructor_part const&) = default;
            storage_mconstructor_part(storage_mconstructor_part&& other) = default;

            storage_mconstructor_part& operator =(storage_mconstructor_part const&) = default;
            storage_mconstructor_part& operator =(storage_mconstructor_part&&) = default;
        };

        template<typename... Ts>
        struct storage_mconstructor_part<false, Ts...> : storage_cconstructor_part<(std::is_trivially_copy_constructible_v<Ts> && ...), Ts...> {

            using storage_cconstructor_part<(std::is_trivially_copy_constructible_v<Ts> && ...), Ts...>::storage_cconstructor_part;

            storage_mconstructor_part(storage_mconstructor_part const&) = default;

            storage_mconstructor_part(storage_mconstructor_part&& other) noexcept((std::is_nothrow_move_constructible_v<Ts> && ...)) {
                this->construct(*this, std::move(other));
            }

            storage_mconstructor_part& operator =(storage_mconstructor_part const&) = default;
            storage_mconstructor_part& operator =(storage_mconstructor_part&&) = default;
        };

        ////////////////////////////////////// Add copy assinment ///////////////////////////////////////////

        template<typename... Ts>
        struct storage_assigner_part : storage_mconstructor_part<(std::is_trivially_move_constructible_v<Ts> && ...), Ts...> {

            using storage_mconstructor_part<(std::is_trivially_move_constructible_v<Ts> && ...), Ts...>::storage_mconstructor_part;
            using storage_mconstructor_part<(std::is_trivially_move_constructible_v<Ts> && ...), Ts...>::operator =;

            template<std::size_t I, typename... As>
            decltype(auto) emplace(As... args) {
                this->destroy();
                try {
                    return this->indx = I,
                       this->construct_alternative(access::storage_base_helper::get_alternative<I>(*this), std::forward<As>(args)...);
                } catch (...) {
                    this->indx = variant_npos;
                    throw;
                }
            }

            template<std::size_t I, typename T, typename A>
            void assign_alternative(alternative<I, T>& a, A&& arg) {
                if (this->indx == I) {
                    a.value = std::forward<A>(arg);
                } else {
                    this->indx = variant_npos;
                    try {
                        if constexpr (std::is_nothrow_constructible_v<T, A> ||
                                      !std::is_nothrow_move_constructible_v<T>) {
                            this->emplace<I>(std::forward<A>(arg));
                        } else {
                            this->emplace<I>(T(std::forward<A>(arg)));
                        }
                    } catch (...) {
                        this->indx = variant_npos;
                        throw;
                    }
                }
            }

            template<typename T>
            void assign(T&& rhs) {
                if (this->valueless_by_exception() && rhs.valueless_by_exception()) return ;
                if (rhs.valueless_by_exception()) {
                    this->destroy();
                    return ;
                }
                visitor::visit_alt_at(rhs.index(),
                                      [this](auto& rhs, auto&& lhs) {
                                          this->assign_alternative(rhs, std::forward<decltype(lhs)>(lhs).value);
                                      }, *this, std::forward<T>(rhs));
            }
        };




        ////////////////////////////////////// Add copy assinment ///////////////////////////////////////////

        template<bool d, typename... Ts>
        struct storage_cassignment_part;

        template<typename... Ts>
        struct storage_cassignment_part<true, Ts...> : storage_assigner_part<Ts...>  {


            using storage_assigner_part<Ts...>::storage_assigner_part;
            using storage_assigner_part<Ts...>::operator =;

            storage_cassignment_part(storage_cassignment_part const&) = default;
            storage_cassignment_part(storage_cassignment_part&&) = default;

            storage_cassignment_part& operator =(storage_cassignment_part const&) = default;
            storage_cassignment_part& operator =(storage_cassignment_part&&) = default;

        };

        template<typename... Ts>
        struct storage_cassignment_part<false, Ts...> : storage_assigner_part<Ts...> {

            storage_cassignment_part(storage_cassignment_part const&) = default;
            storage_cassignment_part(storage_cassignment_part&&) = default;

            using storage_assigner_part<Ts...>::storage_assigner_part;
            using storage_assigner_part<Ts...>::operator =;

            storage_cassignment_part &operator=(const storage_cassignment_part &other) {
                      this->assign(other);
                      return *this;
            }

            storage_cassignment_part& operator =(storage_cassignment_part&&) = default;

        };

        ////////////////////////////////////// Add move assinment ///////////////////////////////////////////

        template<bool d, typename... Ts>
        struct storage_massignment_part;

        template<typename... Ts>
        struct storage_massignment_part<true, Ts...> : storage_cassignment_part<(std::is_trivially_copy_assignable_v<Ts> && ...), Ts...>  {


            using storage_cassignment_part<(std::is_trivially_copy_assignable_v<Ts> && ...), Ts...>::storage_cassignment_part;
            using storage_cassignment_part<(std::is_trivially_copy_assignable_v<Ts> && ...), Ts...>::operator =;

            storage_massignment_part(storage_massignment_part const&) = default;
            storage_massignment_part(storage_massignment_part&&) = default;

            storage_massignment_part& operator =(storage_massignment_part const&) = default;
            storage_massignment_part& operator =(storage_massignment_part&& other) = default;

        };

        template<typename... Ts>
        struct storage_massignment_part<false, Ts...> : storage_cassignment_part<(std::is_trivially_copy_assignable_v<Ts> && ...), Ts...> {

            storage_massignment_part(storage_massignment_part const&) = default;
            storage_massignment_part(storage_massignment_part&&) = default;

            using storage_cassignment_part<(std::is_trivially_copy_assignable_v<Ts> && ...), Ts...>::storage_cassignment_part;
            using storage_assigner_part<Ts...>::operator =;

            storage_massignment_part &operator=(const storage_massignment_part &other) = default;
            storage_massignment_part& operator =(storage_massignment_part&& other) {
                this->assign(std::move(other));
                return *this;
            }

        };

        /////////////////////////////////////////////////////////////////////////////////

        template<typename... Ts>
        struct storage_t : storage_massignment_part<(std::is_trivially_move_assignable_v<Ts> && ...), Ts...> {

            using storage_massignment_part<(std::is_trivially_move_assignable_v<Ts> && ...), Ts...>::storage_massignment_part;

            storage_t(storage_t const&) = default;
            storage_t(storage_t&&) = default;

            storage_t& operator =(storage_t const&) = default;
            storage_t& operator =(storage_t&&) = default;

            inline void swap(storage_t &other) {
                    if (this->valueless_by_exception() && other.valueless_by_exception()) {
                      // do nothing.
                    } else if (this->index() == other.index()) {
                      visitor::visit_alt_at(this->index(), [](auto &a, auto &b) { std::swap(a.value,b.value); }, *this, other);
                    } else {
                      storage_t *lhs = this;
                      storage_t *rhs = &other;

                      if ((lhs->valueless_by_exception() || std::array<bool, sizeof...(Ts)>{{std::is_nothrow_move_constructible<Ts>::value...}}[lhs->index()]) &&
                          !(rhs->valueless_by_exception() || std::array<bool, sizeof...(Ts)>{{std::is_nothrow_move_constructible<Ts>::value...}}[rhs->index()])) {
                            std::swap(lhs, rhs);
                      }

                      storage_t tmp(std::move(*rhs));

                      try {
                        this->construct(*rhs, std::move(*lhs));
                      } catch (...) {
                          if (tmp.valueless_by_exception() || std::array<bool, sizeof...(Ts)>{{std::is_nothrow_move_constructible<Ts>::value...}}[tmp.index()]) {
                              this->construct(*rhs, std::move(tmp));
                          }
                          throw;
                      }

                      this->construct(*lhs, std::move(tmp));
                    }
            }

        };
} // storage end

} // details end

/////////////////////////////////////////////////////// Non-member functions //////////////////////////////////////////////////////////////////////////

template <class Visitor, class... Variants>
constexpr decltype(auto) visit(Visitor&& vis, Variants&&... vars) {
    return details::visitor::variant_helper::visit_value(std::forward<Visitor>(vis), std::forward<Variants>(vars)...);
}


/////////////////////////////////////////////////////////////////////////

template <typename T, typename... Ts>
constexpr bool holds_alternative(variant<Ts...> const& v) noexcept {
    return details::get_id_at_v<T, Ts...> == v.index() ;
}

/////////////////////////////////////////////////////////////////////////


template <std::size_t I, class... Types>
constexpr variant_alternative_t<I, variant<Types...>>& get(variant<Types...>& v) {
    if (v.index() == I)
        return details::access::variant_helper::get_alternative<I>(v).value;
//    throw bad_variant_access();
}

template <std::size_t I, class... Types>
constexpr variant_alternative_t<I, variant<Types...>>&& get(variant<Types...>&& v) {
    if (v.index() == I)
        return details::access::variant_helper::get_alternative<I>(std::move(v)).value;
//    throw bad_variant_access();
}

template <std::size_t I, class... Types>
constexpr variant_alternative_t<I, variant<Types...>> const& get(variant<Types...> const& v) {
    if (v.index() == I)
        return details::access::variant_helper::get_alternative<I>(v).value;
//    throw bad_variant_access();
}

template <std::size_t I, class... Types>
constexpr variant_alternative_t<I, variant<Types...>> const&& get(variant<Types...> const&& v) {
    if (v.index() == I)
        return details::access::variant_helper::get_alternative<I>(std::move<variant<Types...>>(v)).value;
//    throw bad_variant_access();
}

///

template <class T, class... Types, std::size_t I = details::get_id_at_v<T, Types...>>
constexpr T& get(variant<Types...>& v) {
//    std::cout << I << std::endl;
    return get<I>(v);
//    return details::access::variant_helper::get_alternative<I>(v).value;
}

template <class T, class... Types, std::size_t I = details::get_id_at_v<T, Types...>>
constexpr T&& get(variant<Types...>&& v) {
    return get<I>(std::move(v));
//    return details::access::variant_helper::get_alternative<I>(std::move(v)).value;
}

template <class T, class... Types, std::size_t I = details::get_id_at_v<T, Types...>>
constexpr const T& get(variant<Types...> const& v) {
    return get<I>(v);
//    return details::access::variant_helper::get_alternative<I>(v).value;
}

template <class T, class... Types, std::size_t I = details::get_id_at_v<T, Types...>>
constexpr const T&& get(variant<Types...> const&& v) {
    return get<I>(std::move(v));
//    return details::access::variant_helper::get_alternative<I>(std::move<variant<Types...>>(v)).value;
}


/////////////////////////////////////////////////////////////////////////

template <std::size_t I, class... Types>
constexpr std::add_pointer_t<variant_alternative_t<I, variant<Types...>>> get_if(variant<Types...>* pv) noexcept {
    if (!pv) return nullptr;
    if (pv->index() == I) {
        return &details::access::variant_helper::get_alternative<I>(*pv).value;
    }
    return nullptr;
}

template <std::size_t I, class... Types>
constexpr std::add_pointer_t<const variant_alternative_t<I, variant<Types...>>> get_if(const variant<Types...>* pv) noexcept {
    if (!pv) return nullptr;
    if (pv->index() == I) {
        return &details::access::variant_helper::get_alternative<I>(*pv).value;
    }
    return nullptr;
}

template <class T, class... Types>
constexpr std::add_pointer_t<T> get_if(variant<Types...>* pv) noexcept {
    constexpr std::size_t I = details::get_id_at_v<T, Types...>;
    if (!pv) return nullptr;
    if (pv->index() == I) {
        return &details::access::variant_helper::get_alternative<I>(*pv).value;
    }
    return nullptr;
}

template <class T, class... Types>
constexpr std::add_pointer_t<const T> get_if(const variant<Types...>* pv) noexcept {
    constexpr std::size_t I = details::get_id_at_v<T, Types...>;
    if (!pv) return nullptr;
    if (pv->index() == I) {
        return &details::access::variant_helper::get_alternative<I>(*pv).value;
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////

template <class... Types>
constexpr bool operator==(const variant<Types...>& v, const variant<Types...>& w) {
    if (v.index()!= w.index()) return false;
    if (v.valueless_by_exception()) return true;
    return get<v.index()>(v) == get<v.index()>(w);
}

template <class... Types>
constexpr bool operator!=(const variant<Types...>& v, const variant<Types...>& w) {
    if (v.index() != w.index()) return true;
    if (v.valueless_by_exception()) return false;
    return get<v.index()>(v) != get<v.index()>(w);
}

template <class... Types>
constexpr bool operator<(const variant<Types...>& v, const variant<Types...>& w) {
    if (w.valueless_by_exception()) return false;
    if (v.valueless_by_exception()) return true;
    if (v.index() < w.index()) return true;
    if (v.index() > w.index()) return false;
    return std::get<v.index()>(v) < std::get<v.index()>(w);
}

template <class... Types>
constexpr bool operator>(const variant<Types...>& v, const variant<Types...>& w) {
    if (v.valueless_by_exception()) return true;
    if (w.valueless_by_exception()) return false;
    if (v.index() < w.index()) return false;
    if (v.index() > w.index()) return true;
    return std::get<v.index()>(v) > std::get<v.index()>(w);
}

template <class... Types>
constexpr bool operator<=(const variant<Types...>& v, const variant<Types...>& w) {
    if (v.valueless_by_exception()) return true;
    if (w.valueless_by_exception()) return false;
    if (v.index() < w.index()) return true;
    if (v.index() > w.index()) return false;
    return get<v.index()>(v) <= get<v.index()>(w);
}

template <class... Types>
constexpr bool operator>=(const variant<Types...>& v, const variant<Types...>& w) {
    if (w.valueless_by_exception()) return true;
    if (v.valueless_by_exception()) return false;
    if (v.index() > w.index()) return true;
    if (v.index() < w.index()) return false;
    return get<v.index()>(v) >= get<v.index()>(w);
}

//////////////////////////////////////////////////////////////

template <class... Types>
void swap(variant<Types...>& lhs, variant<Types...>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

/////////////////////////////////////////////////////// END Non-member functions //////////////////////////////////////////////////////////////////////////


template<typename... Ts>
struct variant {

    static_assert(0 < sizeof...(Ts), "variant must have at least one alternative");

//////////////////////////// Constructors ///////////////////////////////////

    /* 1 */
    template<std::enable_if_t<std::is_default_constructible_v<details::get_type_at_t<0, Ts...>>>* = nullptr>
    constexpr variant() noexcept(std::is_nothrow_default_constructible_v<details::get_type_at_t<0, Ts...>>)
        : storage(std::in_place_index_t<0>{}) {}

    /* 2 */
    constexpr variant(variant const&) = default;

    /* 3 */
    constexpr variant(variant&&) = default;

    /* 4 */
    template <typename T,
              typename D = std::decay_t<T>,
              std::enable_if_t<!std::is_same_v<D, variant> && !details::is_in_place_v<D> &&
                               !details::is_in_place_index_v<D> && !details::is_in_place_type_v<D>>* = nullptr,
              std::size_t I = details::find_best_match_v<T, Ts...>,
              typename M = details::get_type_at_t<I, Ts...>,
              std::enable_if_t<std::is_constructible_v<M, T>>* = nullptr>
    constexpr variant(T&& arg) noexcept(std::is_nothrow_constructible<M, T>::value)
        : storage(std::in_place_index<I>, std::forward<T>(arg)) {
//         std::cerr << "type constuctor " << I << std::endl;
    }

    /* 5 */
    template <typename T, typename... As,
            std::size_t I = details::get_id_at_v<T, Ts...>,
            std::enable_if_t<std::is_constructible_v<T, As...>>* = nullptr>
    constexpr explicit variant(std::in_place_type_t<T>, As&&... args) noexcept(std::is_nothrow_constructible_v<T, As...>)
        : storage(std::in_place_index<I>, std::forward<As>(args)...) {}

    /* 6 */
    template <typename T, typename U, typename... As,
            std::size_t I = details::get_id_at_v<T, Ts...>,
            std::enable_if_t<std::is_constructible_v<T, std::initializer_list<U>, As...>>* = nullptr>
    constexpr explicit variant(std::in_place_type_t<T>, std::initializer_list<U> il, As&&... args) noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, As...>)
        : storage(std::in_place_index<I>, il, std::forward<As>(args)...) {}

     /* 7 */
    template <std::size_t I, typename... As,
              typename T = details::get_type_at_t<I, Ts...>,
              std::enable_if_t<std::is_constructible_v<T, As...>>* = nullptr>
    constexpr explicit variant(std::in_place_index_t<I>, As&&... args) noexcept(std::is_nothrow_constructible<T, As...>::value)
        : storage(std::in_place_index<I>, std::forward<As>(args)...) {}

    /* 8 */
    template <std::size_t I, typename U, typename... As,
            typename T = details::get_type_at_t<I, Ts...>,
            std::enable_if_t<std::is_constructible_v<T, std::initializer_list<U>, As...>>* = nullptr>
    constexpr explicit variant(std::in_place_index_t<I>, std::initializer_list<U> il, As&&... args) noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, As...>)
        : storage(std::in_place_index<I>, il, std::forward<As>(args)...) {}

//////////////////////////// END Constructors ///////////////////////////////////

    ~variant() = default;

//////////////////////////// Assignment ///////////////////////////////////

    /* 1 */
    variant &operator=(const variant &) = default;

    /* 2 */
    variant &operator=(variant &&) = default;

    /* 3 */
    template <typename A,
                  std::enable_if_t<!std::is_same<std::decay_t<A>, variant>::value>* = nullptr,
                  std::size_t I = details::find_best_match_v<A, Ts...>,
                  typename T = details::get_type_at_t<I, Ts...>,
                  std::enable_if_t<(std::is_assignable_v<T, A> &&std::is_constructible_v<T, A>)>* = nullptr>
    variant &operator=(A&& arg) noexcept(std::is_nothrow_assignable_v<T, A> && std::is_nothrow_constructible_v<T, A>) {
          storage.assign_alternative(details::access::storage_base_helper::get_alternative<I>(storage), std::forward<A>(arg));
          return *this;
    }

////////////////////////////// END Assignment ////////////////////////////////////////////////////////


    template <class T, class... Args,
              std::size_t I = details::get_id_at_v<T, Ts...>,
              std::enable_if_t<std::is_constructible_v<T, Args...>>* = nullptr>
    T& emplace(Args&&... args) {
        return storage.template emplace<I>(std::forward<Args>(args)...);
    }

    template <class T, class U, class... Args,
              std::size_t I = details::get_id_at_v<T, Ts...>,
              std::enable_if_t<std::is_constructible_v<T, std::initializer_list<U>&, Args...>>* = nullptr>
    T& emplace(std::initializer_list<U> il, Args&&... args) {
        storage.template emplace<I>(il, std::forward<Args>(args)...);
    }

    template <size_t I, class... Args,
              std::enable_if_t<std::is_constructible_v<details::get_type_at_t<I, Ts...>, Args...>>* = nullptr>
    variant_alternative_t<I, variant>& emplace(Args&&... args) {
        storage.template emplace<I>(std::forward<Args>(args)...);
    }

    template <size_t I, class U, class... Args,
              std::enable_if_t<std::is_constructible_v<details::get_type_at_t<I, Ts...>, std::initializer_list<U>&, Args...>>* = nullptr>
    variant_alternative_t<I, variant>& emplace(std::initializer_list<U> il, Args&&... args) {
        storage.template emplace<I>(il, std::forward<Args>(args)...);
    }

    constexpr std::size_t index() const noexcept {
        return storage.index();
    }

    constexpr bool valueless_by_exception() const noexcept {
        return storage.valueless_by_exception();
    }

    constexpr static std::size_t size() {
        return sizeof...(Ts);
    }

    void swap(variant& rhs ) noexcept(((std::is_nothrow_move_constructible_v<Ts> &&
                                         std::is_nothrow_swappable_v<Ts>) && ...)) {
        storage.swap(rhs.storage);
    }

    friend struct details::access::variant_helper;
    friend struct details::visitor::variant_helper;
private:

    details::storage::storage_t<Ts...> storage;

};

} // end vr


#endif // VARIANT_H
