#include <bits/stdc++.h>
#include "temp_variant.h"
//#include "variant.h"
#include <variant>

using namespace std;

struct X {
    int x;

    X() = delete;
    X(int x) : x(x) {}
};

template struct my_variant<int, char, string>;

my_variant<int, char, double> w;

int main() {

    my_variant<string, char, double> v;
    cout << v.get<string>() << endl;
    my_variant<int, char, string> c(string("asd"));
    cout << c.get<string>() << endl;
    c = 12;
    cout << c.get<int>() << endl;
//    my_variant<int, char, string> c(1);
    return 0;
}
