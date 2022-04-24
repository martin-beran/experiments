/* Sizes of various C++ types
 *
 * Compile with C++17 or higher
 */

#include <any>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

bool new_called = false;
bool new_log = false;
bool delete_log = false;

void* operator new(std::size_t sz)
{
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

template <class T> void display_size(std::string_view type)
{
    std::string_view pref{"decltype("};
    std::string_view suf{"{})"};
    if (type.size() > pref.size() + suf.size()) {
        if (type.substr(0, pref.size()) == pref)
            type.remove_prefix(pref.size());
        if (type.substr(type.size() - suf.size()) == suf)
            type.remove_suffix(suf.size());
    }
    std::cout << "sizeof(" << type << ")=" << sizeof(T) << std::endl;
}

#define DISPLAY_SIZE(type) display_size<type>(#type)

template <class T> void display_realloc(std::string_view type,
                                        size_t n = 1'000'000)
{
    T o{};
    std::cout << type << " initial capacity " << o.capacity() <<
        " reallocations at sizes:" << std::endl;
    unsigned reallocs = 0;
    for (size_t i = 1; i <= n; ++i) {
        new_called = false;
        o.push_back(typename T::value_type{});
        if (new_called)
            std::cout << ' ' << ++reallocs << ':' << i << "->" << o.capacity();
    }
    std::cout << std::endl;
}

#define DISPLAY_REALLOC(type) display_realloc<type>(#type)

template <class T> class alloc_c: public std::allocator<T> {
    using std::allocator<T>::allocator;
public:
    char c;
};

template <class T> class alloc_l: public std::allocator<T> {
    using std::allocator<T>::allocator;
public:
    long l;
};

int main(int, char*[])
{
    // sizes of types
    DISPLAY_SIZE(char);
    DISPLAY_SIZE(short);
    DISPLAY_SIZE(int);
    DISPLAY_SIZE(long);
    DISPLAY_SIZE(long long);
    DISPLAY_SIZE(void*);
    DISPLAY_SIZE(std::string);
    DISPLAY_SIZE(std::string_view);
    DISPLAY_SIZE(std::optional<char>);
    DISPLAY_SIZE(std::optional<long long>);
    // preprocessor does not handle the comma in
    // DISPLAY_SIZE(std::variant<char, unsigned char>);
    DISPLAY_SIZE(decltype(std::variant<char, unsigned char>{}));
    DISPLAY_SIZE(decltype(std::variant<char, long long, std::string>{}));
    DISPLAY_SIZE(std::unique_ptr<char>);
    DISPLAY_SIZE(std::shared_ptr<char>);
    DISPLAY_SIZE(std::weak_ptr<char>);
    DISPLAY_SIZE(std::atomic<int>);
    DISPLAY_SIZE(std::mutex);
    DISPLAY_SIZE(std::condition_variable);
    DISPLAY_SIZE(std::thread);
    DISPLAY_SIZE(std::vector<int>);
    DISPLAY_SIZE(decltype(std::map<std::string, int>{}));
    DISPLAY_SIZE(decltype(std::unordered_map<std::string, int>{}));
    // reallocations in std containers
    DISPLAY_REALLOC(std::string);
    DISPLAY_REALLOC(std::vector<char>);
    DISPLAY_REALLOC(std::vector<long>);
    // allocations related to std::shared_ptr
    new_log = true;
    delete_log = true;
    {
        std::cout << "new/delete" << std::endl;
        auto p = new bool(true);
        delete p;
    }
    {
        std::cout << "make_shared()" << std::endl;
        auto p = std::make_shared<bool>(true);
    }
    {
        std::cout << "shared_ptr(new())" << std::endl;
        std::shared_ptr<bool> p{new bool(true)};
    }
    {
        std::cout << "allocate_shared(), allocator without state" << std::endl;
        std::allocator<char> alloc;
        auto p4 = std::allocate_shared<bool>(alloc, true);
    }
    {
        std::cout << "allocate_shared(), allocator with char state" <<
            std::endl;
        alloc_c<char> alloc;
        auto p5 = std::allocate_shared<bool>(alloc, true);
    }
    {
        std::cout << "allocate_shared(), allocator with long state" <<
            std::endl;
        alloc_l<char> alloc;
        auto p5 = std::allocate_shared<bool>(alloc, true);
    }
    return 0;
}
