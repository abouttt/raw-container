#pragma once

#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <type_traits>

namespace raw::detail
{

[[noreturn]] inline void assert_fail(
	const char* expr,
	const char* msg,
	const std::source_location& loc = std::source_location::current()) noexcept
{

	std::fprintf(stderr, "[RAW ASSERT] %s:%u: %s%s%s\n", loc.file_name(), loc.line(), (msg ? msg : ""), (msg ? " " : ""), expr);
	std::abort();
}

} // namespace raw::detail

#ifndef NDEBUG
#define RAW_ASSERT(expr, ...) \
        (std::is_constant_evaluated() ? \
            ((expr) ? void(0) : throw "Assertion failed") : \
            ((expr) ? void(0) : ::raw::detail::assert_fail(#expr, "" __VA_ARGS__, std::source_location::current())))
#else
#define RAW_ASSERT(expr, ...) ((void)0)
#endif
