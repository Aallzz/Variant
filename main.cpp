#include <bits/stdc++.h>
#include "variant.h"
#include <variant>

struct stype {
    int x {};
    double g {};
    std::vector<int> v {};
    std::string s {};

    stype(int x, double g, std::string s) : x(x), g(g), v{x, static_cast<int>(g)}, s(s) {}

    template<typename T>
    stype(std::initializer_list<T> lst) : x(static_cast<int>(lst.size())) {}
};

struct stype_constexpr {
    int y;

    constexpr stype_constexpr(int y) : y(y) {}

    ~stype_constexpr() = default;
};

#define COMPLEX_TYPES_WITH_USER_DEFINED int,double,std::string,stype
#define COMPLEX_TYPES int,double,std::string
#define CONSTEXPR_TYPES int,double
#define CONSTEXPR_TYPES_WITH_USER_DEFINED int,double,stype_constexpr

int main() {

    std::string test1;
    vr::variant<COMPLEX_TYPES> v(std::string("asdasd"));
    test1 += std::to_string(v.index());
    vr::variant<COMPLEX_TYPES> vv(1.0);
    test1 += std::to_string(vv.index());
    vr::variant<COMPLEX_TYPES> vvv(std::move(v));
    test1 += std::to_string(vvv.index());
    std::cout << test1 << std::endl; // ans = 212

    std::string test2;
    constexpr vr::variant<CONSTEXPR_TYPES> q(1.1);
    test2 += std::to_string(q.index());
    constexpr vr::variant<CONSTEXPR_TYPES> qq(q);
    test2 += std::to_string(qq.index());
    constexpr vr::variant<CONSTEXPR_TYPES> qqq(std::move(q));
    test2 += std::to_string(qqq.index());
    test2 += std::to_string(q.index());
    std::cout << test2 << std::endl; // ans = 111

    std::string test3;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> g(std::in_place_index<3>, 1, 11.4, std::string("Heeellllooooo"));
    test3 += std::to_string(g.index());
    std::cout << test3 << std::endl;   // ans = 3

    std::string test4;
    constexpr vr::variant<CONSTEXPR_TYPES_WITH_USER_DEFINED> gg(std::in_place_index<2>, 11);
    test4 += std::to_string(gg.index());
    std::cout << test4 << std::endl; // ans = 2

    std::string test5;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> w(std::in_place_type<stype>, 1, 11.4, std::string("Heeellllooooo"));
    test5 += std::to_string(w.index());
    std::cout << test5 << std::endl;   // ans = 3

    std::string test6;
    constexpr vr::variant<CONSTEXPR_TYPES_WITH_USER_DEFINED> ww(std::in_place_type<stype_constexpr>, 11);
    test6 += std::to_string(ww.index());
    std::cout << test6 << std::endl; // ans = 2

    std::string test7;
    constexpr vr::variant<CONSTEXPR_TYPES> r(std::in_place_type<int>, 11);
    test7 += std::to_string(r.index());
    constexpr vr::variant<CONSTEXPR_TYPES> rr(std::in_place_type<double>, 1.1);
    test7 += std::to_string(rr.index());
    std::cout << test7 << std::endl; // ans 01

    std::string test8;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> www(std::in_place_type<stype>, {1, 1, 1});
    test8 += std::to_string(www.index());
    std::cout << test8 << std::endl;   // ans = 3

    std::string test9;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> wwww(std::in_place_index<3>, {1, 1, 1});
    test9 += std::to_string(wwww.index());
    std::cout << test9 << std::endl;   // ans = 3



    std::string test10;
    vr::variant<COMPLEX_TYPES> c(123);
    test10 += std::to_string(c.index());
    vr::variant<COMPLEX_TYPES> cc(std::string("asd"));
    test10 += std::to_string(cc.index());
    c = cc;
    test10 += std::to_string(c.index());
    std::cout << test10 << std::endl;   // ans = 022

    std::string test11;
    vr::variant<CONSTEXPR_TYPES> x(123);
    test11 += std::to_string(x.index());
    vr::variant<CONSTEXPR_TYPES> xx(2.0);
    test11 += std::to_string(xx.index());
    x = xx;
    test11 += std::to_string(x.index());
    std::cout << test11 << std::endl;   // ans = 011


    std::string test12;
    vr::variant<COMPLEX_TYPES> a(123);
    test12 += std::to_string(a.index());
    a = vr::variant<COMPLEX_TYPES>(std::string("asd"));
    test12 += std::to_string(c.index());
    std::cout << test12 << std::endl;   // ans = 02

    std::string test13;
    vr::variant<CONSTEXPR_TYPES> b(123);
    test13 += std::to_string(b.index());
    b = vr::variant<CONSTEXPR_TYPES> (2.0);;
    test13 += std::to_string(b.index());
    std::cout << test13 << std::endl;   // ans = 01

    std::string test14;
    int a14 = 13;
    double b14 = 14;
    vr::variant<COMPLEX_TYPES> e(a14);
    test14 += std::to_string(e.index());
    e = b14;
    test14 += std::to_string(e.index());
    e = std::string("asd");
    test14 += std::to_string(e.index());
    std::cout << test14 << std::endl;   // ans = 012

    std::string test15;
    vr::variant<CONSTEXPR_TYPES> ee(123);
    test15 += std::to_string(ee.index());
    ee = 12.1;
    test15 += std::to_string(ee.index());
    ee = 1;
    test15 += std::to_string(ee.index());
    std::cout << test15<< std::endl;   // ans = 010

    std::string test16;
    vr::variant<COMPLEX_TYPES> t;
    std::string sss = "abacaba";
    t = sss;
    test16 += std::to_string(vr::holds_alternative<std::string>(t));
    test16 += std::to_string(vr::holds_alternative<stype>(t));
    test16 += std::to_string(vr::holds_alternative<int>(t));
    t = 1.0;
    test16 += std::to_string(vr::holds_alternative<std::vector<int>>(t));
    test16 += std::to_string(vr::holds_alternative<double>(t));
    std::cout << test16 << std::endl;  // ans = 10001
//    vr::variant<int, double, std::string> vvvv(s{});
    return 0;
}
