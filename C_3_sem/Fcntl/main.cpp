#include <iostream>
#include <compare>

struct cl
{
    int x = 0;
    auto operator<=>(const cl &lhs) const = default;
};

int main()
{
    cl var, va2;
    std::cout << std::boolalpha << (var == va2);
}