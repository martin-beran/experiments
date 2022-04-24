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

template <class, class = void> struct has_member_v: std::false_type {};
template <class T> struct has_member_v<T, std::void_t<decltype(&T::v)>>:
    std::true_type {};

template <class, class = void> struct has_member_f: std::false_type {};
template <class T> struct has_member_f<T, std::void_t<decltype(&T::f)>>:
    std::true_type {};

#define DEF_HAS_MEMBER(m) \
    template <class, class = void> \
    struct m_has_member_##m: std::false_type {}; \
    template <class T> \
    struct m_has_member_##m<T, std::void_t<decltype(&T::m)>>: \
        std::true_type {};

DEF_HAS_MEMBER(v)
DEF_HAS_MEMBER(f)

#ifndef NO_CONCEPTS

template <class T> inline constexpr bool c_has_member_v = requires { &T::v; };
template <class T> inline constexpr bool c_has_member_f = requires { &T::f; };

template <class T> using tid = std::type_identity<T>;

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

    std::cout << "No concepts with macros" << std::endl;

    std::cout << "s_none::v=" << m_has_member_v<s_none>::value << std::endl;
    std::cout << "s_v::v=" << m_has_member_v<s_v>::value << std::endl;
    std::cout << "s_f::v=" << m_has_member_v<s_f>::value << std::endl;
    std::cout << "s_v_f::v=" << m_has_member_v<s_v_f>::value << std::endl;

    std::cout << "s_none::f=" << m_has_member_f<s_none>::value << std::endl;
    std::cout << "s_v::f=" << m_has_member_f<s_v>::value << std::endl;
    std::cout << "s_f::f=" << m_has_member_f<s_f>::value << std::endl;
    std::cout << "s_v_f::f=" << m_has_member_f<s_v_f>::value << std::endl;

#ifndef NO_CONCEPTS

    std::cout << "With concepts" << std::endl;

    std::cout << "s_none::v=" << c_has_member_v<s_none> << std::endl;
    std::cout << "s_v::v=" << c_has_member_v<s_v> << std::endl;
    std::cout << "s_f::v=" << c_has_member_v<s_f> << std::endl;
    std::cout << "s_v_f::v=" << c_has_member_v<s_v_f> << std::endl;

    std::cout << "s_none::f=" << c_has_member_f<s_none> << std::endl;
    std::cout << "s_v::f=" << c_has_member_f<s_v> << std::endl;
    std::cout << "s_f::f=" << c_has_member_f<s_f> << std::endl;
    std::cout << "s_v_f::f=" << c_has_member_f<s_v_f> << std::endl;

    // This method allows defining l_has_member_* locally in a function
    std::cout << "With concepts and lambdas" << std::endl;
    auto has_member_v =
        [](auto v) { return requires { &decltype(v)::type::v; }; };
    auto has_member_f =
        [](auto v) { return requires { &decltype(v)::type::f; }; };

    std::cout << "s_none::v=" << has_member_v(tid<s_none>{}) << std::endl;
    std::cout << "s_v::v=" << has_member_v(tid<s_v>{}) << std::endl;
    std::cout << "s_f::v=" << has_member_v(tid<s_f>{}) << std::endl;
    std::cout << "s_v_f::v=" << has_member_v(tid<s_v_f>{}) << std::endl;

    std::cout << "s_none::f=" << has_member_f(tid<s_none>{}) << std::endl;
    std::cout << "s_v::f=" << has_member_f(tid<s_v>{}) << std::endl;
    std::cout << "s_f::f=" << has_member_f(tid<s_f>{}) << std::endl;
    std::cout << "s_v_f::f=" << has_member_f(tid<s_v_f>{}) << std::endl;

#define HAS_MEMBER(t, m) \
    [](auto v) { \
        return requires { &decltype(v)::type::m; }; \
    }(std::type_identity<t>{})

    std::cout << "With concepts, lambdas, and macros" << std::endl;
    std::cout << "s_none::v=" << HAS_MEMBER(s_none, v) << std::endl;
    std::cout << "s_v::v=" << HAS_MEMBER(s_v, v) << std::endl;
    std::cout << "s_f::v=" << HAS_MEMBER(s_f, v) << std::endl;
    std::cout << "s_v_f::v=" << HAS_MEMBER(s_v_f, v) << std::endl;

    std::cout << "s_none::f=" << HAS_MEMBER(s_none, f) << std::endl;
    std::cout << "s_v::f=" << HAS_MEMBER(s_v, f) << std::endl;
    std::cout << "s_f::f=" << HAS_MEMBER(s_f, f) << std::endl;
    std::cout << "s_v_f::f=" << HAS_MEMBER(s_v_f, f) << std::endl;
#endif

    return 0;
}
