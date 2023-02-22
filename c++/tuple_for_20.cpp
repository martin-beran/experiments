/* Call a visitor for each value in a std::tuple
 *
 * Compile with C++20 or higher
 */

#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

#if 0
template <class T, class F> void for_each(T&& t, F&& f)
{
    auto i = std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{};
    auto helper =
        []<class U, size_t... I>(U&& t, auto&& f,
                                 std::index_sequence<I...>)
        {
            (..., f(std::get<I>(std::forward<U>(t))));
        };
    helper(std::forward<T>(t), std::forward<F>(f), i);
}
#endif

template <class T, class F> void for_each(T&& t, F&& f)
{
    []<class U, size_t... I>(U&& t, auto&& f, std::index_sequence<I...>) {
        (..., f(std::get<I>(std::forward<U>(t))));
    }(std::forward<T>(t), std::forward<F>(f),
      std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
}

struct out {
    out() {
        std::cout << '{';
    }
    ~out() {
        std::cout << "}\n";
    }
    template <class T> static constexpr bool printable =
        requires(T v) {
            std::cout << v;
        };
    template <class T> void operator()(T&& v) requires printable<T> {
        std::cout << d << v;
        d = ",";
    };
    template <class T> void operator()(T&&) requires (!printable<T>) {
        std::cout << d << '?';
        d = ",";
    };
    std::string_view d = "";
};

template <class T> void out_tuple(T&& t) {
    for_each(std::forward<T>(t), out{});
}

int main(int argc, char* argv[])
{
    std::ostringstream save;
    save.copyfmt(std::cout);
    std::cout << std::boolalpha;
    out_tuple(std::make_tuple());
    out_tuple(std::make_tuple(0));
    out_tuple(std::make_tuple(true, std::vector<int>{}));
    out_tuple(std::make_tuple("abc", 2, 'x'));
    out_tuple(std::tuple{"abc", "d", 1.23e25, &main, 'x', false});
    std::cout.copyfmt(save);
    out_tuple(std::tuple{"abc", "d", 1.23e25, &main, 'x', false});
    return EXIT_SUCCESS;
}
