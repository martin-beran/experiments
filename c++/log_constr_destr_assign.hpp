#pragma once

/* Logging constructors, destructors, and assignments to cerr
 *
 * Compile with C++17 or higher
 */

#include <iostream>
#include <optional>
#include <string>

template <const char* N> class cda {
public:
    cda() { log(type_name, {}); }
    cda(const cda<N>& o) {
	using namespace std::literals;
	log(type_name, "const "s + type_name + "&"s, &o);
    }
    cda(cda<N>&& o) {
	using namespace std::literals;
	log(type_name, type_name + "&&"s, &o);
    }
    ~cda() {
	using namespace std::literals;
	log("~"s + type_name, ""s);
    }
    cda<N>& operator=(const cda<N>& o) {
	using namespace std::literals;
	log("operator=", "const "s + type_name + "&"s, &o);
	return *this;
    }
    cda<N>& operator=(cda<N>&& o) {
	using namespace std::literals;
	log("operator=", type_name + "&&"s, &o);
	return *this;
    }
protected:
    static constexpr const char* type_name = N;
    void log(const std::string& f, const std::string& a,
	     std::optional<const void*> p = std::nullopt)
    {
	std::cerr << this << "->" << type_name << "::" << f << "(" << a;
	if (p)
	    std::cerr << " " << *p;
	std::cerr << ")" << std::endl;
    }
};
