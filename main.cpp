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

template struct vr::my_variant<int, char, string>;

my_variant<int, char, double> w;

int main() {


    my_variant<string, char, double> v;
    cout << v.get<string>() << endl;
    my_variant<int, char, string> c(string("asd"));
    cout << c.get<string>() << endl;
    c.visit([](auto const& value){cout << value << endl;});
    c = 12;
    cout << c.get<int>() << endl;
    c = "hello"s;


    return 0;
}
