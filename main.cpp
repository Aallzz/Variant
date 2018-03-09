#include <bits/stdc++.h>
#include "variant.h"
#include <variant>
#include "gtest/gtest.h"
#include "tst_adf.h"


struct f {
    constexpr int operator()(int a) {
        return a * a;
    }
    constexpr double operator()(double a) {
        return a * a;
    }
};

int main(int argc, char* argv[]) {

    constexpr vr::variant<int> v(12);
    constexpr int x = vr::get<int>(v);
//    constexpr auto ff = f();
//    constexpr auto x = std::visit(ff, v);
//    std::cout << x << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
