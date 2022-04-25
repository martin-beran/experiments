#pragma once

/* Logging memory allocations and deallocations.
 *
 * Compile with C++17 or higher
 */

#include <cstdlib>
#include <iostream>
#include <new>

namespace new_delete {

extern bool new_called;
extern bool new_log;
extern bool delete_log;

}
