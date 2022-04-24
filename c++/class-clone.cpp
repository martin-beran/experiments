/* A polymorphic class hierarchy which implements cloning objects and permits
 * multiple inheritance.
 *
 * Compile with C++17 or higher
 */

#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

class clonable_base {
protected:
    explicit clonable_base(std::tuple<> = {}) {}
    virtual ~clonable_base() = default;
    virtual std::shared_ptr<clonable_base> clone_impl() = 0;
};

template <class D, class B0 = clonable_base, class ...B>
class clonable: public B0, public B... {
public:
    template <class A0, class ...A> explicit clonable(A0&& a0, A&& ...a):
        B0(std::forward<A0>(a0)), B(std::forward<A>(a))... {}
    std::shared_ptr<D> clone() {
        return std::dynamic_pointer_cast<D>(clone_impl());
    }
protected:
    std::shared_ptr<clonable_base> clone_impl() override {
        return std::dynamic_pointer_cast<B0>(
            std::make_shared<D>(dynamic_cast<D&>(*this))
        );
    }
};

class clonable_int: public clonable<clonable_int> {
public:
    explicit clonable_int(std::tuple<int> i): clonable_int(std::get<0>(i)) {}
    explicit clonable_int(int i = {}): clonable(std::tuple<>{}), i(i) {}
    int i;
};

std::ostream& operator<<(std::ostream& os, const clonable_int& v)
{
    return os << v.i;
}

class clonable_str: public clonable<clonable_str> {
public:
    explicit clonable_str(std::tuple<std::string> s):
        clonable_str(std::get<0>(s)) {}
    explicit clonable_str(const char* s): clonable_str(std::string{s}) {}
    explicit clonable_str(std::string s = {}):
        clonable(std::tuple<>{}), s(std::move(s)) {}
    std::string s;
};

std::ostream& operator<<(std::ostream& os, const clonable_str& v)
{
    return os << v.s;
}

class clonable_int_str:
    public clonable<clonable_int_str, clonable_int, clonable_str>
{
public:
    explicit clonable_int_str(int i = {}, std::string s = {}):
        clonable(std::tuple{i}, std::tuple{std::move(s)}) {}
};

std::ostream& operator<<(std::ostream& os, const clonable_int_str& v)
{
    operator<<(os, static_cast<const clonable_int&>(v));
    os << ',';
    operator<<(os, static_cast<const clonable_str&>(v));
    return os;
}

class clonable_int1: public clonable<clonable_int1, clonable_int> {
    using clonable<clonable_int1, clonable_int>::clonable;
};

class clonable_int2: public clonable<clonable_int2, clonable_int> {
    using clonable<clonable_int2, clonable_int>::clonable;
};

class clonable_int12:
    public clonable<clonable_int12, clonable_int1, clonable_int2>
{
public:
    explicit clonable_int12(int i1 = {}, int i2 = {}):
        clonable(std::tuple{i1}, std::tuple{i2}) {}
};

std::ostream& operator<<(std::ostream& os, const clonable_int12& v)
{
    operator<<(os, static_cast<const clonable_int1&>(v));
    os << ',';
    operator<<(os, static_cast<const clonable_int2&>(v));
    return os;
}

template <class T> void display(T&& v)
{
    auto p = v.clone();
    std::cout << v << ' ' << *p << std::endl;
}

int main(int argc, char* argv[])
{
    display(clonable_int(1));
    display(clonable_str("a"));
    display(clonable_int_str(2, "b"));
    display(clonable_int12(3, 4));
    return 0;
}
