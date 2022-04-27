/* Effects of different memory order
 * This appears to never fail on x86_64. Maybe it would fail on a weak memory
 * architecture (ARM)?
 *
 * Compile with C++20 or higher
 */

#include <atomic>
#include <barrier>
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
using cnt_t = std::atomic<unsigned long long>;
cnt_t cnt_a{0};
data_t data_a{};
cnt_t cnt_b{0};
data_t data_b{};
cnt_t ok{0};
data_t::value_type expected = 1;
static_assert(std::barrier<>::max() >= 4);
constexpr size_t num_threads = 4;
std::barrier end_point(num_threads);
std::barrier restart_point(num_threads);

void f_write(cnt_t& c, std::memory_order mo)
{
    for (;;) {
        c.fetch_add(1, mo);
        end_point.arrive_and_wait();
        restart_point.arrive_and_wait();
    }
}

void f_read(cnt_t& a, cnt_t& b, std::memory_order mo, bool leader)
{
    auto [mr, mw] = mo_rw(mo);
    unsigned long long failures = 0;
    for (;;) {
        while (a.load(mr) != expected)
            ;
        if (b.load(mr) == expected)
            ++ok;
        end_point.arrive_and_wait();
        if (leader) {
            if (ok == 0) {
                ++failures;
                std::cout << "cnt=" << expected << " failures=" << failures <<
                    " rate=" << (double(failures) / expected) << std::endl;
            }
            cnt_a = expected;
            cnt_b = expected;
            ++expected;
            ok = 0;
        }
        restart_point.arrive_and_wait();
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
    std::thread t_write1(f_write, std::ref(cnt_a), mo);
    std::thread t_write2(f_write, std::ref(cnt_b), mo);
    std::thread t_read1(f_read, std::ref(cnt_a), std::ref(cnt_b), mo, true);
    std::thread t_read2(f_read, std::ref(cnt_b), std::ref(cnt_a), mo, false);
    t_write1.join();
    t_write2.join();
    t_read1.join();
    t_read2.join();
    return EXIT_SUCCESS;
}
