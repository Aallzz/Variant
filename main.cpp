#include <bits/stdc++.h>
#include "variant.h"
#include <variant>

using namespace std;
using namespace vr;

struct X {
    int x;

    X() = delete;
    X(int x) : x(x) {}
};

//template struct vr::my_variant<int, char, string>;

my_variant<int, char, double> w;

int main() {


    my_variant<char, double, int> v(1.0);
    my_variant<char, double, int> vv(v);
    my_variant<char, double, int> vvv = vv;
    variant<int, double> x(1);
//    cout << visit([](auto const& value){return value;}, x) << endl;
    cout << v.visit([](auto const& value){return value;}) << endl;
    return 0;
}
