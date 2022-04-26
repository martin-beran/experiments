/* Effects of different memory order
 * On x86_64, this program should be compiled with -fsanitize=thread. When run
 * with argument "relaxed", the thread sanitizer reports a data race. It runs
 * without a data race with other memory orders.
 *
 * Compile with C++17 or higher
 */

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

int usage(std::string_view argv0)
{
    std::cerr << "usage: " << argv0 << " {relaxed|acq_rel|seq_cst}" <<
        std::endl;
    return EXIT_FAILURE;
}

std::pair<std::memory_order, std::memory_order> mo_rw(std::memory_order mo)
{
    auto mr = mo;
    auto mw = mo;
    switch (mo) {
    case std::memory_order_acq_rel:
        mr = std::memory_order_acquire;
        mw = std::memory_order_release;
        break;
    case std::memory_order_relaxed:
    case std::memory_order_consume:
    case std::memory_order_acquire:
    case std::memory_order_release:
    case std::memory_order_seq_cst:
    default:
        break;
    }
    return {mr, mw};
}

constexpr size_t sz = 10000;
using data_t = std::array<unsigned long long, sz>;
data_t data{};
std::atomic<unsigned long long> cnt{0};

void f_prod(std::memory_order mo)
{
    auto [mr, mw] = mo_rw(mo);
    for (size_t i = 1;; ++i) {
        data[i % sz] = i;
        cnt.store(i, mw);
        while (cnt.load(mr) != 0)
            ;
    }
}

void f_cons(std::memory_order mo)
{
    auto [mr, mw] = mo_rw(mo);
    unsigned long long failures = 0;
    for (size_t i = 1;; ++i) {
        decltype(cnt)::value_type c;
        do {
            c = cnt.load(mr);
        } while (c == 0);
        auto d = data[c % sz];
        cnt.store(0, mw);
        if (d != 0 && c > d) {
            ++failures;
            std::cout << "data=" << d << " cnt=" << c <<
                " failures=" << failures <<
                " rate=" << (double(failures) / (i + 1)) << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        return usage(argv[0]);
    std::memory_order mo;
    using namespace std::string_view_literals;
    if (argv[1] == "relaxed"sv)
        mo = std::memory_order_relaxed;
    else if (argv[1] == "acq_rel"sv)
        mo = std::memory_order_acq_rel;
    else if (argv[1] == "seq_cst"sv)
        mo = std::memory_order_seq_cst;
    else
        return usage(argv[0]);
    std::thread t_prod(f_prod, mo);
    std::thread t_cons(f_cons, mo);
    t_prod.join();
    t_cons.join();
    return EXIT_SUCCESS;
}
