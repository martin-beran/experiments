/* Logging constructors, destructors, and assignments to cerr
 *
 * Compile with C++17 or higher
 */

#include "log_constr_destr_assign.hpp"

#include <tuple>

constexpr char test_name[] = "test";

class test: public cda<test_name> {
};

void fv(test o)
{
}

void fr(test& o)
{
}

template <class T> void ff(T&& o)
{
}

struct s_const {
    s_const(const test& t): t(t) {
        std::cout << this << "->s_const(const test&).t==" << &this->t <<
            std::endl;
    }
    test t;
};

struct s_rref {
    s_rref(const test& t): t(t) {
        std::cout << this << "->s_rref(const test&).t==" << &this->t <<
            std::endl;
    }
    s_rref(test&& t): t(std::move(t)) {
        std::cout << this << "->s_rref(test&&).t==" << &this->t << std::endl;
    }
    test t;
};

struct s_value {
    s_value(test t): t(std::move(t)) {
        std::cout << this << "->s_value(test).t=" << &this->t << std::endl;
    }
    test t;
};

int main(int, char*[])
{
    std::cout << "objects" << std::endl;
    {
        std::cout << "create objects" << std::endl;
        test o1;
        test o2;
        std::cout << "create copy" << std::endl;
        test oc = o1;
        std::cout << "create move" << std::endl;
        test om = std::move(o1);
        std::cout << "assign copy" << std::endl;
        o2 = o1;
        std::cout << "assign move" << std::endl;
        o2 = std::move(o1);
        std::cout << "end of scope" << std::endl;
    }

    std::cout << "\nfunction calls" << std::endl;
    {
        test o;
        std::cout << "value" << std::endl;
        fv(o);
        std::cout << "reference" << std::endl;
        fr(o);
        std::cout << "forwarding reference (value)" << std::endl;
        ff(test{});
        std::cout << "forwarding reference (reference)" << std::endl;
        ff(o);
        std::cout << "end of scope" << std::endl;
    }

    std::cout << "\ntuple deduction" << std::endl;
    {
        test o;
        std::cout << "tuple<value, value>" << std::endl;
        std::tuple t{test{}, o};
        static_assert(std::is_same_v<std::tuple_element<0, decltype(t)>::type,
                      test>);
        static_assert(std::is_same_v<std::tuple_element<1, decltype(t)>::type,
                      test>);
        std::cout << "t<" << &std::get<0>(t) << ", " << &std::get<1>(t) <<
          '>' << std::endl;
        std::cout << "end of scope" << std::endl;
    }
    std::cout << "\ntuple explicit types" << std::endl;
    {
        test o;
        std::cout << "tuple<value, reference>" << std::endl;
        std::tuple<test, test&> t{test{}, o};
        static_assert(std::is_same_v<std::tuple_element<0, decltype(t)>::type,
                      test>);
        static_assert(std::is_same_v<std::tuple_element<1, decltype(t)>::type,
                      test&>);
        std::cout << "t<" << &std::get<0>(t) << ", " << &std::get<1>(t) <<
          '>' << std::endl;
        std::cout << "end of scope" << std::endl;
    }
    std::cout << "\nconstructed from rvalue" << std::endl;
    std::cout << "s_const" << std::endl;
    {
        s_const v{test{}};
    }
    std::cout << "s_rref" << std::endl;
    {
        s_rref v{test{}};
    }
    std::cout << "s_value" << std::endl;
    {
        s_value v{test{}};
    }
    std::cout << "\nconstructed from lvalue" << std::endl;
    std::cout << "s_const" << std::endl;
    {
        test t;
        std::cout << "t" << std::endl;
        s_const v{t};
    }
    std::cout << "s_rref" << std::endl;
    {
        test t;
        std::cout << "t" << std::endl;
        s_rref v{t};
    }
    std::cout << "s_value" << std::endl;
    {
        test t;
        std::cout << "t" << std::endl;
        s_value v{t};
    }
    return 0;
}
