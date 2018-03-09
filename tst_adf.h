#ifndef TST_ADF_H
#define TST_ADF_H

#include "gtest/gtest.h"
#include "variant"
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

void c(vr::variant<int>&& , vr::variant<int>&& ) {
    std::cout << "ASDASD" << std::endl;
}

TEST(Construction, from_type) {
    std::string test1;
    vr::variant<COMPLEX_TYPES> v(std::string("asdasd"));
    test1 += std::to_string(v.index());
    vr::variant<COMPLEX_TYPES> vv(1.0);
    test1 += std::to_string(vv.index());
    vr::variant<COMPLEX_TYPES> vvv(std::move(v));
    test1 += std::to_string(vvv.index());
    EXPECT_EQ(test1, "212");
//    std::cout << test1 << std::endl; // ans = 212
}


TEST(Construction, copy_move) {
    std::string test2;
    constexpr vr::variant<CONSTEXPR_TYPES> q(1.1);
    test2 += std::to_string(q.index());
    constexpr vr::variant<CONSTEXPR_TYPES> qq(q);
    test2 += std::to_string(qq.index());
    constexpr vr::variant<CONSTEXPR_TYPES> qqq(std::move(q));
    test2 += std::to_string(qqq.index());
    test2 += std::to_string(q.index());
    EXPECT_EQ(test2, "1111");
//    std::cout << test2 << std::endl; // ans = 1111
}


TEST(Constuction, in_place_index) {
    std::string test3;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> g(std::in_place_index<3>, 1, 11.4, std::string("Heeellllooooo"));
    test3 += std::to_string(g.index());
    EXPECT_EQ(test3, "3");
//    std::cout << test3 << std::endl;   // ans = 3
}

TEST(Construction, in_place_index_1) {
    std::string test4;
    constexpr vr::variant<CONSTEXPR_TYPES_WITH_USER_DEFINED> gg(std::in_place_index<2>, 11);
    test4 += std::to_string(gg.index());
    EXPECT_EQ(test4, "2");
//    std::cout << test4 << std::endl; // ans = 2
}

TEST(Construction, in_place_type) {
    std::string test5;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> w(std::in_place_type<stype>, 1, 11.4, std::string("Heeellllooooo"));
    test5 += std::to_string(w.index());
    EXPECT_EQ(test5, "3");
//    std::cout << test5 << std::endl;   // ans = 3
}

TEST(Construction, in_place_type_1) {
    std::string test6;
    constexpr vr::variant<CONSTEXPR_TYPES_WITH_USER_DEFINED> ww(std::in_place_type<stype_constexpr>, 11);
    test6 += std::to_string(ww.index());
    EXPECT_EQ(test6, "2");
//    std::cout << test6 << std::endl; // ans = 2
}

TEST(Construction, in_place_type_2) {
    std::string test7;
    constexpr vr::variant<CONSTEXPR_TYPES> r(std::in_place_type<int>, 11);
    test7 += std::to_string(r.index());
    constexpr vr::variant<CONSTEXPR_TYPES> rr(std::in_place_type<double>, 1.1);
    test7 += std::to_string(rr.index());
    EXPECT_EQ(test7, "01");
//    std::cout << test7 << std::endl; // ans 01
}

TEST(Construction, in_place_type_li) {
    std::string test8;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> www(std::in_place_type<stype>, {1, 1, 1});
    test8 += std::to_string(www.index());
    EXPECT_EQ(test8, "3");
//    std::cout << test8 << std::endl;   // ans = 3
}

TEST(Construction, in_place_index_li) {
    std::string test9;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> wwww(std::in_place_index<3>, {1, 1, 1});
    test9 += std::to_string(wwww.index());
    EXPECT_EQ(test9, "3");
//    std::cout << test9 << std::endl;   // ans = 3
}


TEST(Assignment, copy) {
    std::string test10;
    vr::variant<COMPLEX_TYPES> c(123);
    test10 += std::to_string(c.index());
    vr::variant<COMPLEX_TYPES> cc(std::string("asd"));
    test10 += std::to_string(cc.index());
    c = cc;
    test10 += std::to_string(c.index());
    EXPECT_EQ(test10, "022");
//    std::cout << test10 << std::endl;   // ans = 022
}

TEST(Assignment, copy_1) {
    std::string test11;
    vr::variant<CONSTEXPR_TYPES> x(123);
    test11 += std::to_string(x.index());
    vr::variant<CONSTEXPR_TYPES> xx(2.0);
    test11 += std::to_string(xx.index());
    x = xx;
    test11 += std::to_string(x.index());
    EXPECT_EQ(test11, "011");
//    std::cout << test11 << std::endl;   // ans = 011
}

TEST(Assignment, move) {
    std::string test12;
    vr::variant<COMPLEX_TYPES> a(123);
    test12 += std::to_string(a.index());
    a = vr::variant<COMPLEX_TYPES>(std::string("asd"));
    test12 += std::to_string(a.index());
    EXPECT_EQ(test12, "02");
//    std::cout << test12 << std::endl;   // ans = 02
}

TEST(Assignment, move_1) {
    std::string test13;
    vr::variant<CONSTEXPR_TYPES> b(123);
    test13 += std::to_string(b.index());
    b = vr::variant<CONSTEXPR_TYPES> (2.0);;
    test13 += std::to_string(b.index());
    EXPECT_EQ(test13, "01");
//    std::cout << test13 << std::endl;   // ans = 01
}

TEST(Assignment, move_copy_type) {
    std::string test14;
    int a14 = 13;
    double b14 = 14;
    vr::variant<COMPLEX_TYPES> e(a14);
    test14 += std::to_string(e.index());
    e = b14;
    test14 += std::to_string(e.index());
    e = std::string("asd");
    test14 += std::to_string(e.index());
    EXPECT_EQ(test14, "012");
//    std::cout << test14 << std::endl;   // ans = 012
}

TEST(Assignment, move_literal) {
    std::string test15;
    vr::variant<CONSTEXPR_TYPES> ee(123);
    test15 += std::to_string(ee.index());
    ee = 12.1;
    test15 += std::to_string(ee.index());
    ee = 1;
    test15 += std::to_string(ee.index());
    EXPECT_EQ(test15, "010");
//    std::cout << test15<< std::endl;   // ans = 010
}

TEST(Holds_alternative, type) {
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
    EXPECT_EQ(test16, "10001");
//    std::cout << test16 << std::endl;  // ans = 10001
}

TEST(Get, index) {
    std::string test17;
    vr::variant<COMPLEX_TYPES> u(11);
    int u1 = vr::get<0>(u);
    test17 += std::to_string(u1) + " ";
    u = std::string("hello");
    test17 += vr::get<2>(u) + " ";
    vr::variant<COMPLEX_TYPES> uu(vr::variant<COMPLEX_TYPES>(2.0));
    test17 += std::to_string(uu.index()) + " " + std::to_string(vr::get<1>(uu)) + " ";
    u = uu;
    test17 += std::to_string(vr::get<1>(u));
    test17 += vr::get<2>(vr::variant<COMPLEX_TYPES> (std::in_place_index<2>, " bye"));
    EXPECT_EQ(test17, "11 hello 1 2.000000 2.000000 bye");
//    std::cout << test17 << std::endl;  // ans = 11 hello 1 2.0 2.0 bye
}

TEST(Get, type) {
    std::string test18;
    vr::variant<COMPLEX_TYPES> y(11);
    int y1 = vr::get<int>(y);
    test18 += std::to_string(y1) + " ";
    y = std::string("hello");
    test18 += vr::get<std::string>(y) + " ";
    vr::variant<COMPLEX_TYPES> yy(vr::variant<COMPLEX_TYPES>(2.0));
    test18 += std::to_string(yy.index()) + " " + std::to_string(vr::get<double>(yy)) + " ";
    y = yy;
    test18 += std::to_string(vr::get<double>(y));
    test18 += vr::get<std::string>(vr::variant<COMPLEX_TYPES> (std::in_place_type<std::string>, " bye"));
    EXPECT_EQ(test18, "11 hello 1 2.000000 2.000000 bye");
//    std::cout << test18 << std::endl;  // ans = 11 hello 1 2.0 2.0 bye
}

TEST(Get, exception) {
    std::string test19;
    constexpr vr::variant<CONSTEXPR_TYPES> i;
    try {
        test19 += std::to_string(vr::get<double>(i));
    } catch(vr::bad_variant_access& e) {
        test19 += "BAD";
    }
    EXPECT_EQ(test19, "BAD");
//    std::cout << test19 << std::endl; // ans = BAD
}

TEST(Visit, one_arg) {
    std::string test20;
    vr::visit([&test20](auto&& a) {
        test20 += std::to_string(1 + a);
    }, vr::variant<CONSTEXPR_TYPES>(std::in_place_index<0>, 11));
    EXPECT_EQ(test20, "12");
//    std::cout << test20 << std::endl; // ans = 12
}


TEST(Visit, two_arg) {
    vr::variant<CONSTEXPR_TYPES> ll(11);
    vr::variant<CONSTEXPR_TYPES> l(11);

    std::string test21;
    vr::visit([&test21](auto&& a, auto&& b) {
        test21 += std::to_string(b + a);
    },
    l,
    ll
    );
    EXPECT_EQ(test21, "22");
//    std::cout << test21 << std::endl; // ans 22
}

TEST(Get_if, trivial) {
    std::string test22;
    vr::variant<int, float> f{12};
    if(auto pval = vr::get_if<int>(&f)) {
      test22 += std::string("variant value: ");
      test22 += std::to_string(*pval);
    } else
      test22 += "failed to get value!";
    EXPECT_EQ(test22, "variant value: 12");
//    std::cout << test22 << std::endl;
}

TEST(Swap, trivial) {
    std::string test24;
    vr::variant<CONSTEXPR_TYPES> f1{1};
    vr::variant<CONSTEXPR_TYPES> f2{2};
    test24 += std::to_string(vr::get<0>(f1)) + std::to_string(vr::get<0>(f2)) + "\n";
    swap(f1, f2);
    test24 += std::to_string(vr::get<0>(f1)) + std::to_string(vr::get<0>(f2));
    EXPECT_EQ(test24, "12\n21");
//    std::cout << test24 << std::endl;
}

TEST(Emplace, trivial) {
    std::string test25;
    vr::variant<COMPLEX_TYPES_WITH_USER_DEFINED> f3;
    test25 += std::to_string(vr::get<0>(f3));
    f3.emplace<stype>(9, 0, std::string("asd"));
    test25 += vr::get<stype>(f3).s;
    EXPECT_EQ(test25, "0asd");
//    std::cout << test25 << std::endl;
}

#endif // TST_ADF_H
