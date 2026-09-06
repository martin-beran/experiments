/* Computing various functions using Taylor series compared to implementations
 * from the standard library
 */

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cxxabi.h>
#include <exception>
#include <map>
#include <memory>
#include <print>
#include <stdexcept>
#include <string_view>
#include <typeinfo>
#include <variant>
#include <vector>

namespace {

struct cmdline_t {
    std::string_view arg0;
    std::vector<std::string_view> args;
    cmdline_t(const int argc, const char* const argv[]):
        arg0(argv[0]), args(argv + 1, argv + argc) {}
};

template<class T> T get_arg(std::string_view s) {
    T res;
    auto [ptr, ec] = std::from_chars(s.begin(), s.end(), res);
    if (ec != std::errc{})
        throw std::system_error(std::make_error_code(ec));
    else if (ptr != s.end())
        throw std::runtime_error("Unparsed characters after number");
    return res;
}

using float_val = std::variant<float, double, long double>;
using fun_type = float_val (*)(float_val, std::optional<float_val>);

#define INIT_FUN_MAP1(f) {#f, fun_impl{&std_fun1<f>, &std_fun1<static_cast<T (*)(T)>(&std::f)>, false}},
#define INIT_FUN_MAP2(f) {#f, fun_impl{&std_fun2<f>, &std_fun2<static_cast<T (*)(T, T)>(&std::f)>, true}},

#define INIT_TYPE_MAP(t) {#t, [](std::string_view f){ \
    decltype(auto) fi = fun_impl<t>::get(f); \
    return fun{fi.fun, fi.std_fun, fi.has_arg2, fun_impl<t>::get_arg}; \
}},

template<class T> class fun_impl {
    template<auto F> static float_val std_fun1(float_val a, std::optional<float_val>) {
        return F(std::get<T>(a));
    }
    template<auto F> static float_val std_fun2(float_val a, std::optional<float_val> b) {
        return F(std::get<T>(a), std::get<T>(*b));
    }
    static T sqrt(T a) { return a; }
    static T cbrt(T a) { return a; }
    static T pow(T a, T b) { return a + b; }
    static T sin(T a) { return a; }
    static T cos(T a) { return a; }
    static T tan(T a) { return a; }
    static T asin(T a) { return a; }
    static T acos(T a) { return a; }
    static T atan(T a) { return a; }
    static T exp(T a) { return a; }
    static T exp2(T a) { return a; }
    static T log(T a) { return a; }
    static T log10(T a) { return a; }
    static T log2(T a) { return a; }
public:
    static const fun_impl& get(std::string_view name) {
        static const std::map<std::string_view, fun_impl> f{
            INIT_FUN_MAP1(sqrt)
            INIT_FUN_MAP1(cbrt)
            INIT_FUN_MAP2(pow)
            INIT_FUN_MAP1(sin)
            INIT_FUN_MAP1(cos)
            INIT_FUN_MAP1(tan)
            INIT_FUN_MAP1(asin)
            INIT_FUN_MAP1(acos)
            INIT_FUN_MAP1(atan)
            INIT_FUN_MAP1(exp)
            INIT_FUN_MAP1(exp2)
            INIT_FUN_MAP1(log)
            INIT_FUN_MAP1(log10)
            INIT_FUN_MAP1(log2)
        };
        if (auto it = f.find(name); it != f.end())
            return it->second;
        else
            throw std::runtime_error(std::string{"Unknown function "}.append(name));
    }
    static float_val get_arg(std::string_view s) {
        return ::get_arg<T>(s);
    }
    const fun_type fun;
    const fun_type std_fun;
    const bool has_arg2;
private:
    fun_impl(fun_type fun, fun_type std_fun, bool has_arg2): fun(fun), std_fun(std_fun), has_arg2(has_arg2) {}
};

class type_impl {
public:
    struct fun {
        fun_type f;
        fun_type std_f;
        bool has_arg2;
        float_val (*get_arg)(std::string_view);
    };
    static const fun get(std::string_view type, std::string_view name) {
        static const std::map<std::string_view, fun (*)(std::string_view)> t{
            INIT_TYPE_MAP(float)
            INIT_TYPE_MAP(double)
            INIT_TYPE_MAP(long double)
        };
        if (auto it = t.find(type); it != t.end())
            return it->second(name);
        else 
            throw std::runtime_error(std::string{"Unknown type "}.append(type));
    }
};

struct args_t {
    std::string_view fun_s;
    std::string_view type_s;
    type_impl::fun fun;
    float_val arg1;
    std::optional<float_val> arg2;
    unsigned terms;
    std::optional<unsigned> iter;
};

void usage(std::string_view arg0)
{
    std::println(R"(usage: {} function type arg1 [arg2] terms [iter]

function:
    sqrt arg1
    cbrt arg1
    pow arg1 arg2
    sin arg1
    cos arg1
    tan arg1
    asin arg1
    acos arg1
    atan arg1
    exp arg1
    exp2 arg1
    log arg1
    log10 arg1
    log2 arg1

type:
    float
    double
    longdouble

terms: number of terms of Taylor series

iter: do this number of iterations and measure time
)", arg0);
}

args_t process_cmdline(const cmdline_t& cmdline)
{
    args_t args{};
    auto fail = [&cmdline](std::string_view msg = {}) {
        usage(cmdline.arg0);
        std::string what{"Invalid command line arguments"};
        if (!msg.empty())
            what.append(": ").append(msg);
        if (std::current_exception())
            std::throw_with_nested(std::runtime_error(what));
        else
            throw std::runtime_error(what);
    };
    size_t arg_i = 0;
    if (cmdline.args.size() <= arg_i)
        fail("Missing fun");
    args.fun_s = cmdline.args[arg_i];
    if (cmdline.args.size() <= ++arg_i)
        fail("Missing type");
    args.type_s = cmdline.args[arg_i];
    try {
        args.fun = type_impl::get(args.type_s, args.fun_s);
    } catch (...) {
        fail();
    }
    if (cmdline.args.size() <= ++arg_i)
        fail("Missing arg1");
    try {
        args.arg1 = args.fun.get_arg(cmdline.args[arg_i]);
    } catch (...) {
        fail("Invalid arg1");
    }
    if (args.fun.has_arg2) {
        if (cmdline.args.size() <= ++arg_i)
            fail("Missing arg2");
        try {
            args.arg2 = args.fun.get_arg(cmdline.args[arg_i]);
        } catch (...) {
            fail("Invalid arg2");
        }
    }
    if (cmdline.args.size() <= ++arg_i)
        fail("Missing terms");
    try {
        args.terms = get_arg<decltype(args.terms)>(cmdline.args[arg_i]);
        if (args.terms <= 0)
            fail("Terms must be greater than 0");
    } catch (...) {
        fail("Invalid terms");
    }
    if (cmdline.args.size() > ++arg_i)
        try {
            args.iter = get_arg<decltype(args.iter)::value_type>(cmdline.args[arg_i]);
            if (args.iter <= 0)
                fail("Iter must be greter than 0");
        } catch (...) {
            fail("Invalid iter");
        }
    if (cmdline.args.size() > ++arg_i)
        fail("Too many arguments");
    return args;
}

std::string demangle(const char* name)
{
    int status = 0;
    if (std::unique_ptr<char, decltype([](char* p) { std::free(p); })> dname{
        abi::__cxa_demangle(name, nullptr, nullptr, &status)})
    {
        try {
            std::string res{dname.get()};
            return res;
        } catch(...) {
            std::throw_with_nested( std::runtime_error("Cannot convert demangled name to std::string"));
        }
    } else {
        std::string_view reason = "unknown";
        switch (status) {
        case 0:
            reason = "success";
            break;
        case -1:
            reason = "memory allocation failure";
            break;
        case -2:
            reason = "invalid mangled name";
            break;
        case -3:
            reason = "invalid argument";
            break;
        default:
            break;
        }
        throw std::runtime_error((std::string{"Cannot demangle "} + name + ": ").append(reason));
    }
}

void print_exception(const std::exception& e, unsigned level = 0)
{
    std::println(stderr, "{0}exception {1}: {2}", std::string(level, ' '), demangle(typeid(e).name()), e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& nested) {
        print_exception(nested, level + 2);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    try {
        cmdline_t cmdline(argc, argv);
        auto args = process_cmdline(cmdline);
        (void) args;
    } catch (const std::exception& e) {
        std::print(stderr, "Terminated by ");
        print_exception(e);
        return EXIT_FAILURE;
    } catch (...) {
        std::println(stderr, "unknown exception");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
