#define CATCH_CONFIG_MAIN

#include <exception>
#include <catch.hpp>

namespace std {
	inline bool uncaught_exception() noexcept(true) {return current_exception() != nullptr;}
}

// Leave this file empty

// For testing documentation, visit https://github.com/philsquared/Catch
