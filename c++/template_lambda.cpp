/* A lambda that is itself a template, not just having a template operator() */

#include <iostream>
#include <list>
#include <set>
#include <vector>

template <template <class ...> class T, class C>
auto container_convert_f(C&& c)
{
    using r_type = T<typename std::decay_t<C>::value_type>;
    r_type r;
    for (auto&&v: c)
        if constexpr (requires () { r.push_back(v); })
            r.push_back(v);
        else if constexpr (requires() { r.insert(v); })
            r.insert(v);
        else {
            //static_assert(false); // does not work in g++-12, clang++-15
            static_assert(false && sizeof(C) > 1); // false dependent on C
        }
    return r;
}

template <template <class ...> class T> constexpr auto container_convert_l =
    []<class C>(C&& c) {
        return container_convert_f<T>(std::forward<C>(c));
    };

template <class T> auto convert(auto&& converter, T&& c)
{
    return converter(std::forward<T>(c));
}

void print(std::string_view prefix, auto&& c)
{
    std::cout << prefix;
    prefix = " ";
    for (auto&& v: c)
        std::cout << prefix << v;
    std::cout << std::endl;
}

int main()
{
    std::list<int> l{1, 2, 3, 4, 5};
    // cannot pass function template (not all parameters of
    // container_convert_f specified)
    //auto bad = convert(container_convert_f<std::set>, l);
    auto s = convert(container_convert_l<std::set>, l);
    print("s:", s);
    auto v = convert(container_convert_l<std::vector>, l);
    print("v:", v);
    return 0;
}
