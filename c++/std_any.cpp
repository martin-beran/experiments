/* Features of std::any
 *
 * Compile with C++17 or higher
 */

#include "new_delete.hpp"
#include "log_constr_destr_assign.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <iostream>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

inline constexpr char empty_name[] = "empty";
struct empty: cda<empty_name> {
};
static_assert(std::is_nothrow_move_constructible_v<empty>);


std::ostream& operator<<(std::ostream& o, const empty&)
{
    return o << "(empty)";
}

inline constexpr char data_int_name[] = "data_int";
struct data_int: cda<data_int_name> {
    data_int(int i = {}): i(i) {}
    int i;
};
static_assert(std::is_nothrow_move_constructible_v<data_int>);

std::ostream& operator<<(std::ostream& o, const data_int& v)
{
    return o << v.i;
}

inline constexpr char data_big_name[] = "data_big";
struct data_big: cda<data_big_name> {
    data_big(int i = {}) {
        std::fill(a.begin(), a.end(), i);
    }
    std::array<int, 1024> a;
};
static_assert(std::is_nothrow_move_constructible_v<data_big>);

std::ostream& operator<<(std::ostream& o, const data_big& v)
{
    return o << v.a.size() << "x" << v.a[0];
}

template <class T, class F>
std::pair<const std::type_index, std::function<void(const std::any&)>>
make_any_visitor(const F& f)
{
    return {
        std::type_index(typeid(T)),
        [f](const std::any & a) {
            if constexpr (std::is_void_v<T>)
                f(a);
            else
                f(a, std::any_cast<const T&>(a));
        }
    };
}

void display_any_obj(const char* t, const std::any& a)
{
    std::cout << "any<" << t << ">=" << & a << " sizeof=" << sizeof(a) <<
        " has_value=" << a.has_value();
}

void display_val(const char* t, const std::any& a)
{
    display_any_obj(t, a);
    std::cout << std::endl;
}

template <class T>
void display_val(const char* t, const std::any& a, const T& v)
{
    display_any_obj(t, a);
    std::cout << " value=" << v << " @" << &v << std::endl;
}

#define MAKE_DISPLAY_VISITOR(t) \
    make_any_visitor<t>( \
        [](const auto& a, const auto& v) { display_val(#t, a, v); })

std::unordered_map<std::type_index, std::function<void(const std::any&)>>
display_visitors{
    make_any_visitor<void>([](auto&& a){ display_val("void", a); }),
    MAKE_DISPLAY_VISITOR(empty),
};

void display(const std::any& a)
{
    if (auto it = display_visitors.find(std::type_index(a.type()));
        it != display_visitors.end())
    {
        it->second(a);
    } else {
        display_any_obj(a.type().name(), a);
        std::cout << " unregistered type" << std::endl;
    }
}

template <size_t S> bool create_any()
{
    new_delete::new_called = false;
    (void)std::any(std::array<char, S>{});
    return new_delete::new_called;
}

template <size_t ...I> size_t check_any_alloc(std::index_sequence<I...>)
{
    size_t min_alloc = 0;
    (void)(... && (create_any<I>() ? (min_alloc = I, false) : true));
    return min_alloc;
}

int main() {
    new_delete::new_log = true;
    new_delete::delete_log = true;
    std::any data;
    display(data);
    std::cout << "--- empty" << std::endl;
    data = empty{};
    display(data);
    std::cout << "--- small" << std::endl;
    data = data_int{123};
    display(data);
    std::cout << "--- big" << std::endl;
    data = data_big(4);
    display(data);
    std::cout << "--- std::shared_ptr" << std::endl;
    data = std::shared_ptr<int>{};
    display(data);
    std::cout << "--- end" << std::endl;
    data.reset();
    std::cout << "--- check alloc" << std::endl;
    new_delete::new_log = false;
    new_delete::delete_log = false;
    std::cout << "minimum size of separate allocation=" <<
        check_any_alloc(std::make_index_sequence<128>{}) << std::endl;
    return 0;
}
