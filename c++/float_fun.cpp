/* Computing floating point functions using various approximation methods,
 * compared to implementations from the standard library
 */

#include <cassert>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cxxabi.h>
#include <exception>
#include <limits>
#include <map>
#include <memory>
#include <numbers>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <variant>
#include <vector>

namespace {

/*** Generic utilities *******************************************************/

// Convert from text to a (numeric) value
template<class T> T get_arg(std::string_view s) {
    T res;
    auto [ptr, ec] = std::from_chars(s.begin(), s.end(), res);
    if (ec != std::errc{})
        throw std::system_error(std::make_error_code(ec));
    else if (ptr != s.end())
        throw std::runtime_error("Unparsed characters after number");
    return res;
}

/*** Defitions of functions and types ****************************************/

// Supported floating point types
using float_val = std::variant<float, double, long double>;

// Convert a variant of floating point types to string
std::string to_string(float_val f)
{
    return std::visit([]<class T>(T v){
        return std::format("{0:+.{1}e}", v, std::numeric_limits<T>::max_digits10);
    }, f);
}

// Floating point function with one or two arguments and number of approximation steps
using fun_type = float_val (*)(float_val, std::optional<float_val>, unsigned steps);

// Floating point function with one or two arguments
using std_fun_type = float_val (*)(float_val, std::optional<float_val>);

// Switch according to the requested function
#define INIT_FUN_STR(f) #f
#define INIT_FUN_MAP1(f, var) \
    {INIT_FUN_STR(f ## var), fun_impl{&fun1<f ## var>, &std_fun1<static_cast<T (*)(T)>(&std::f)>, false}},
#define INIT_FUN_MAP2(f, var) \
    {INIT_FUN_STR(f ## var) , fun_impl{&fun2<f ## var>, &std_fun2<static_cast<T (*)(T, T)>(&std::f)>, true}},

template<class T> class fun_impl {
    template<auto F> static float_val fun1(float_val a, std::optional<float_val>, unsigned steps) {
        return F(std::get<T>(a), steps);
    }
    template<auto F> static float_val std_fun1(float_val a, std::optional<float_val>) {
        return F(std::get<T>(a));
    }
    template<auto F> static float_val fun2(float_val a, std::optional<float_val> b, unsigned steps) {
        return F(std::get<T>(a), std::get<T>(*b), steps);
    }
    template<auto F> static float_val std_fun2(float_val a, std::optional<float_val> b) {
        return F(std::get<T>(a), std::get<T>(*b));
    }
    static T sqrt_newton(T a, unsigned steps);
    static T sqrt_taylor(T a, unsigned steps);
    static T cbrt(T a, unsigned steps);
    static T pow(T a, T b, unsigned steps);
    static T sin(T a, unsigned steps);
    static T cos(T a, unsigned steps);
    static T tan(T a, unsigned steps);
    static T asin(T a, unsigned steps);
    static T acos(T a, unsigned steps);
    static T atan(T a, unsigned steps);
    static T exp(T a, unsigned steps);
    static T exp2(T a, unsigned steps);
    static T log(T a, unsigned steps);
    static T log10(T a, unsigned steps);
    static T log2(T a, unsigned steps);
public:
    static const fun_impl& get(std::string_view name) {
        static const std::map<std::string_view, fun_impl> f{
            INIT_FUN_MAP1(sqrt, _newton)
            INIT_FUN_MAP1(sqrt, _taylor)
            INIT_FUN_MAP1(cbrt,)
            INIT_FUN_MAP2(pow,)
            INIT_FUN_MAP1(sin,)
            INIT_FUN_MAP1(cos,)
            INIT_FUN_MAP1(tan,)
            INIT_FUN_MAP1(asin,)
            INIT_FUN_MAP1(acos,)
            INIT_FUN_MAP1(atan,)
            INIT_FUN_MAP1(exp,)
            INIT_FUN_MAP1(exp2,)
            INIT_FUN_MAP1(log,)
            INIT_FUN_MAP1(log10,)
            INIT_FUN_MAP1(log2,)
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
    const std_fun_type std_fun;
    const bool has_arg2;
private:
    fun_impl(fun_type fun, std_fun_type std_fun, bool has_arg2): fun(fun), std_fun(std_fun), has_arg2(has_arg2) {}
};

// Switch according to the requested floating point type
#define INIT_TYPE_MAP(t) {#t, [](std::string_view f){ \
    decltype(auto) fi = fun_impl<t>::get(f); \
    return fun{fi.fun, fi.std_fun, fi.has_arg2, fun_impl<t>::get_arg}; \
}},

class type_impl {
public:
    using longdouble = long double;
    struct fun {
        fun_type f;
        std_fun_type std_f;
        bool has_arg2;
        float_val (*get_arg)(std::string_view);
    };
    static const fun get(std::string_view type, std::string_view name) {
        static const std::map<std::string_view, fun (*)(std::string_view)> t{
            INIT_TYPE_MAP(float)
            INIT_TYPE_MAP(double)
            INIT_TYPE_MAP(longdouble)
        };
        if (auto it = t.find(type); it != t.end())
            return it->second(name);
        else 
            throw std::runtime_error(std::string{"Unknown type "}.append(type));
    }
};

/*** Implementation of mathematical functions ********************************/

/* Newton's (Heron's) method:
 *
 *       --                    1      x
 * y = \/ x = lim  y :  y    = - (y + -- )
 *            n->oo n    n+1   2   n  y
 *                                     n
 */
template<class T> T fun_impl<T>::sqrt_newton(T a, unsigned steps)
{
    T res = a;
    for (unsigned i = 0; i < steps; ++i)
        res = T{1.0} / T{2.0} * (res + a / res);
    return res;
}

/* Approximation by Taylor series:
 *
 *           oo
 *          ---     n-1
 *   ---     \  (-1)   (2n)!   n
 * \/1+x  =  /  ------------- x   converges for -1 < x < 1, 0 < 1+x < 2
 *          /    n    2
 *          --- 4 (n!) (2n-1)
 *          n=0
 */
template<class T> T fun_impl<T>::sqrt_taylor(T a, unsigned steps)
{
    if (a < 0.0)
        return -NAN;
    if (a == 0.0)
        return 0;
    bool inv = a > 1.0;
    T x = (inv ? T{1.0} / a : a) - T{1.0};
    T res = 0.0;
    T s = -1.0;
    T f = 1.0;
    T xn = 1.0;
    T n4 = 1.0;
    for (unsigned n = 0; n < steps; ++n) {
        if (n > 0) {
            // f *= (T{2.0} * T(n) - T{1.0}) * T{2.0} * T(n) / T(n) * T(n);
            f *= (T{4.0} * T(n) - T{2.0}) / T(n);
        }
        res += s * f / n4 / (T{2.0} * T(n) - T{1.0}) * xn;
        s = -s;
        xn *= x;
        n4 *= T{4.0};
    }
    return inv ? T{1.0} / res : res;
}

/* Newton's method:
 *
 *     3  --                    1        x
 * y =  \/ x = lim  y :  y    = - (2y + --- )
 *             n->oo n    n+1   3    n    2
 *                                       y
 *                                        n
 */
template<class T> T fun_impl<T>::cbrt(T a, unsigned steps)
{
    T res = a;
    for (unsigned i = 0; i < steps; ++i)
        res = T{1.0} / T{3.0} * (2 * res + a / (res * res));
    return res;
}

template<class T> T fun_impl<T>::pow(T /*a*/, T /*b*/, unsigned /*steps*/)
{
    return NAN;
}

/* Approximation by Taylor series:
 *
 *          oo
 *         ---       n
 *          \    (-1)     2n+1
 * sin x =  /  --------- x
 *         /   (2n + 1)!
 *         ---
 *         n=0
 */
template<class T> T fun_impl<T>::sin(T a, unsigned steps)
{
    a = std::fmod(a, T{2.0} * std::numbers::pi_v<T>);
    T res = 0.0;
    T t = a;
    for (unsigned n = 1; n <= steps; ++n) {
        res += t;
        t = -t * a * a / ((T{2.0} * T(n)) * (T{2.0} * T(n) + T{1.0}));
    }
    return res;
}

/* Approximation by Taylor series:
 *
 *          oo
 *         ---     n
 *          \  (-1)   2n
 * cos x =  /  ----- x
 *         /   (2n)!
 *         ---
 *         n=0
 */
template<class T> T fun_impl<T>::cos(T a, unsigned steps)
{
    a = std::fmod(a, T{2.0} * std::numbers::pi_v<T>);
    T res = 0.0;
    T t = 1.0;
    for (unsigned n = 1; n <= steps; ++n) {
        res += t;
        t = -t * a * a / ((T{2.0} * T(n) - T{1.0}) * (T{2.0} * T(n)));
    }
    return res;
}

template<class T> T fun_impl<T>::tan(T a, unsigned steps)
{
    return sin(a, steps) / cos(a, steps);
}

template<class T> T fun_impl<T>::asin(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

template<class T> T fun_impl<T>::acos(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

template<class T> T fun_impl<T>::atan(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

template<class T> T fun_impl<T>::exp(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

template<class T> T fun_impl<T>::exp2(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

template<class T> T fun_impl<T>::log(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

template<class T> T fun_impl<T>::log10(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

template<class T> T fun_impl<T>::log2(T /*a*/, unsigned /*steps*/)
{
    return NAN;
}

/*** Command line processing *************************************************/

// Unparsed command line arguments
struct cmdline_t {
    std::string_view arg0;
    std::vector<std::string_view> args;
    cmdline_t(const int argc, const char* const argv[]):
        arg0(argv[0]), args(argv + 1, argv + argc) {}
};

// Parsed command line arguments
struct args_t {
    std::string_view fun_s; // function name
    std::string_view type_s; // type name
    type_impl::fun fun; // function
    float_val arg1; // the first argument (all functions)
    std::optional<float_val> arg2; // the second argument (some functions)
    unsigned steps; // the number of approximation steps
    std::optional<unsigned long> iter; // the number of iterations (speed measurement)
};

// Print help
void usage(std::string_view arg0)
{
    std::println(R"(usage: {} function type arg1 [arg2] steps [iter]

function:
    sqrt_newton arg1 (Newton's/Heron's method)
    sqrt_taylor arg1 (Taylor series)
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

steps: number of approximation steps (0 for an initial approximation)

iter: do this number of iterations and measure time
)", arg0);
}

// Parse command line arguments
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
        fail("Missing steps");
    try {
        args.steps = get_arg<decltype(args.steps)>(cmdline.args[arg_i]);
    } catch (...) {
        fail("Invalid steps");
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

/*** Diagnostics *************************************************************/

// Demangling C++ names
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

// Print information from nested exceptions
void print_exception(const std::exception& e, unsigned level = 0)
{
    std::println(stderr, "{0}exception {1}: {2}", std::string(level, ' '), demangle(typeid(e).name()), e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& nested) {
        print_exception(nested, level + 2);
    }
}

/*** Test mathematical functions *********************************************/

// Run function once, analyze precision
void run_precision(const args_t& args)
{
    size_t steps_sz = std::format("{}", args.steps).size();
    float_val std_res = args.fun.std_f(args.arg1, args.arg2);
    for (unsigned i = 0; i <= args.steps; ++i) {
        float_val res = args.fun.f(args.arg1, args.arg2, i);
        float_val diff = std::visit([](auto s, auto r) -> float_val { return s - r; }, res, std_res);
        float_val rel_diff = std::visit([](auto d, auto s) -> float_val { return d / s; }, diff, std_res);
        std::println("{0:{1}} {2} {3} {4}", i, steps_sz, to_string(res), to_string(diff), to_string(rel_diff));
    }
    std::println("{0:{1}} {2}", "", steps_sz, to_string(std_res));
}

// Run function many times, analyze speed
void run_time(const args_t& args)
{
    assert(args.iter > 0);
    auto measure = [&args](std::string_view label, auto f) {
        float_val last;
        auto t0 = std::chrono::steady_clock::now();
        for (unsigned i = 0; i < args.iter; ++i) {
            float_val res;
            if constexpr (requires { f(args.arg1, args.arg2); })
                res = args.fun.std_f(args.arg1, args.arg2);
            else
                res = f(args.arg1, args.arg2, args.steps);
            if (i > 0 && res != last)
                throw std::runtime_error("Function value differs from previous iteration");
            last = res;
        }
        auto t1 = std::chrono::steady_clock::now();
        auto t = t1 - t0;
        std::println("{0}({1}{2}) = {3}",
                     label,
                     to_string(args.arg1),
                     args.arg2 ? ", " + to_string(*args.arg2) : "",
                     to_string(last));
        std::println("{0} {1} * {2:%S} s = {3:%S} s", label, *args.iter, t / decltype(t)::rep(*args.iter), t);
        return t;
    };
    auto t_std = measure("std", args.fun.std_f);
    auto t_fun = measure("fun", args.fun.f);
    std::println("fun / std = {}", double(t_fun.count()) / double(t_std.count()));
}

} // namespace

/*** Main function ***********************************************************/

int main(int argc, char* argv[])
{
    try {
        cmdline_t cmdline(argc, argv);
        auto args = process_cmdline(cmdline);
        if (args.iter)
            run_time(args);
        else
            run_precision(args);
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
