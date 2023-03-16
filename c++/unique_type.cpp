/* A template that generates a unique type on each use */

#include <type_traits>

template <class T> struct same {
    T val;
};

template <class T, auto = []{}> struct unique {
    T val;
};

using same1 = same<int>;
using same2 = same<int>;
static_assert(std::is_same_v<same1, same2>);

using unique1 = unique<int>;
using unique2 = unique<int>;
static_assert(!std::is_same_v<unique1, unique2>);

int main()
{
    same1 s1{1};
    same2 s2 = s1;
    unique1 u1{1};
    // unique2 u2 = u1; // error, different type, no viable conversion exists
    return 0;
}
