/* Implementation part of new_delete.hpp
 *
 * Compile with C++17 or higher
 */

#include "new_delete.hpp"

namespace new_delete {

bool new_called = false;
bool new_log = false;
bool delete_log = false;

}

void* operator new(std::size_t sz)
{
    using namespace new_delete;
    new_called = true;
    static bool recursive = false;
    void* p = malloc(sz);
    if (!recursive && new_log) {
        recursive = true;
        try {
            std::cout << "new(" << sz << ")=" << p << std::endl;
        } catch (...) {
        }
        recursive = false;
    }
    if (!p)
        throw std::bad_alloc{};
    return p;
}

void operator delete(void* p) noexcept
{
    using namespace new_delete;
    static bool recursive = false;
    if (!recursive && delete_log) {
        recursive = true;
        try {
            std::cout << "delete(" << p << ")" << std::endl;
        } catch (...) {
        }
        recursive = false;
    }
    free(p);
}
