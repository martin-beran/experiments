/* Test if a class has a member.
 *
 * The methods based on concepts need at least C++20, other methods need
 * C++17.
 */

#include <iostream>
#include <type_traits>

struct s_none {
};

struct s_v {
    int v;
};

struct s_f {
    int f(int i) {
        return i + 2;
    }
};

struct s_v_f {
    int f(bool b) {
        return b ? 1 : -1;
    }
    unsigned v;
};

template <class T, class = void> struct has_member_v: std::false_type {};
template <class T> struct has_member_v<T, std::void_t<decltype(&T::v)>>:
    std::true_type {};

template <class T, class = void> struct has_member_f: std::false_type {};
template <class T> struct has_member_f<T, std::void_t<decltype(&T::f)>>:
    std::true_type {};

#ifndef NO_CONCEPTS
#endif // NO_CONCEPTS

int main()
{
    std::cout << "No concepts" << std::endl;

    std::cout << "s_none::v=" << has_member_v<s_none>::value << std::endl;
    std::cout << "s_v::v=" << has_member_v<s_v>::value << std::endl;
    std::cout << "s_f::v=" << has_member_v<s_f>::value << std::endl;
    std::cout << "s_v_f::v=" << has_member_v<s_v_f>::value << std::endl;

    std::cout << "s_none::f=" << has_member_f<s_none>::value << std::endl;
    std::cout << "s_v::f=" << has_member_f<s_v>::value << std::endl;
    std::cout << "s_f::f=" << has_member_f<s_f>::value << std::endl;
    std::cout << "s_v_f::f=" << has_member_f<s_v_f>::value << std::endl;

#ifndef NO_CONCEPTS
    std::cout << "With concepts" << std::endl;
#endif

    return 0;
}
