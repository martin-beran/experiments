/* Tuples passed as arguments to a constructor of a derived template class are
 * transformed to constructor arguments of base classes.
 *
 * Compile with C++17 or higher
 */

#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

struct a {
    a(int i = {}, std::string s = {}): i(i), s(std::move(s)) {}
    int i;
    std::string s;
};

std::ostream& operator<<(std::ostream& o, const a& v)
{
    return o << "i=" << v.i << " s=" << v.s;
}

struct b {
    b(char c = {}, double d = {}): c(c), d(d) {}
    char c;
    double d;
};

std::ostream& operator<<(std::ostream& o, const b& v)
{
    return o << "c=" << v.c << " d=" << v.d;
}

template <class B> struct tuple_construct: B {
    template <class ...A> tuple_construct(std::tuple<A...> t):
        tuple_construct(std::index_sequence_for<A...>{}, t) {}
    template <class T, size_t ...I>
    tuple_construct(std::index_sequence<I...>, T& t):
        B(std::get<I>(t)...) {}
};

template <class ...B> struct data: tuple_construct<B>... {
    template <class ...A>
    data(A&& ...a): tuple_construct<B>(std::forward<A>(a))... {}
};

template <class ...B>
std::ostream& operator<<(std::ostream& o, const data<B...>& v)
{
    size_t i = 0;
    auto write = [&i, &o](auto&& v) {
        if (i++ != 0)
            o << ' ';
        o << v;
        return 0;
    };
   (..., (write(static_cast<const B&>(v))));
    return o;
}

int main()
{
    data<a, b> d{std::tuple{1, std::string{"a"}}, std::tuple{'b', 2.3}};
    std::cout << d << std::endl;
    return 0;
}
