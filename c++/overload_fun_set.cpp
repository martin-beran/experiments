/* A named set of overloaded functions
 *
 * Compile with C++20 or higher
 */

#include <iostream>

template <class ...T> struct overloaded: T... {
    using T::operator()...;
};
// needed by clang++-15, clang++-17, not needed by g++-11, g++-12
template <class ...T> overloaded(T&& ...) -> overloaded<T...>;

int main()
{
    overloaded f{
        [](int i) -> int { std::cout << i << std::endl; return i + 1; },
        [](float f) -> int { std::cout << f << std::endl; return 0; },
    };
    std::cout << f(3) << std::endl;
    std::cout << f(2.0f) << std::endl;
}
