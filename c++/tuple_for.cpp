/* Call a visitor for each value in a std::tuple
 *
 * Compile with C++17 or higher
 */

#include <array>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template <class F, class T, size_t ...I>
void tuple_for_impl(F&& f, T&& t, std::index_sequence<I...>)
{
    (..., f(std::get<I>(std::forward<T>(t))));
}

template <class F, class T> void tuple_for(F&& f, T&& t)
{
    tuple_for_impl(std::forward<F>(f), std::forward<T>(t),
                   std::make_index_sequence<std::tuple_size<T>::value>{});
}

struct display {
    template <class T> void operator()(const T& v) {
        std::cout << (first ? "" : ", ") << v;
        first = false;
    }
    bool first = true;
};

struct size {
    size_t value = 0;
    template <class T, class = size_t> struct has_size: std::false_type {};
    template <class T>
    struct has_size<T, decltype(std::declval<T>().size())>: std::true_type {};
    template <class T> auto operator()(const T& v) ->
        std::enable_if_t<has_size<T>::value>
    {
        value += v.size();
    }
    template <class T> auto operator()(const T&) ->
        std::enable_if_t<!has_size<T>::value>
    {
    }
};

struct size2 {
    size_t value = 0;
    template <class T> void operator()(const T& v) {
        if constexpr (size::has_size<T>::value)
            value += v.size();
    }
};

template <class T, T ...I> auto range(std::integer_sequence<T, I...>)
{
    return std::array{I...};
}

struct with_size {
    with_size(size_t sz) noexcept: sz(sz) {}
    size_t size() const noexcept {
        return sz;
    }
    size_t sz;
};

int main()
{
    tuple_for(display(), std::tuple{'?', 1, 2.0, std::string{"abc"}});
    std::cout << std::endl;
    size sz;
    tuple_for(sz, std::tuple{
        1, // 0
        std::string{"abc"}, // 3
        std::vector<int>{1, 2} // 2
    }); // 5
    std::cout << sz.value << std::endl;
    size2 sz2;
    tuple_for(sz2, std::tuple{
        1, // 0
        std::string{"abc"}, // 3
        std::vector<int>{1, 2}, // 2
        range(std::make_integer_sequence<int, 10>{}), // 10
        with_size{6} // 6
    }); // 21
    std::cout << sz2.value << std::endl;
    return 0;
}
