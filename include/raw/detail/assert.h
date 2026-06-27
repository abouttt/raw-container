#pragma once

#include <cstdlib>
#include <format>
#include <iostream>
#include <source_location>
#include <type_traits>

#if defined(_MSC_VER)
#define RAW_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define RAW_DEBUG_BREAK() __builtin_trap()
#else
#define RAW_DEBUG_BREAK() ((void)0)
#endif

namespace raw::detail
{

inline void assert_failed_msg(const char* expr, const std::source_location& loc)
{
	std::cerr << loc.file_name() << "(" << loc.line() << "): Assertion '" << expr << "' failed.\n";
}

template <typename... Args>
inline void assert_failed_msg(const char* expr, const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
{
	std::cerr << loc.file_name() << "(" << loc.line() << "): Assertion '" << expr << "' failed. ";
	std::cerr << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
}

} // namespace raw::detail

#ifndef NDEBUG
#define RAW_ASSERT(expr, ...) \
    do { \
        if (std::is_constant_evaluated()) { \
            if (!(expr)) { \
                throw "Assertion '" #expr "' failed."; \
            } \
        } else { \
            if (!(expr)) [[unlikely]] { \
                ::raw::detail::assert_failed_msg(#expr, std::source_location::current() __VA_OPT__(,) __VA_ARGS__); \
                RAW_DEBUG_BREAK(); \
                std::abort(); \
            } \
        } \
    } while (0)
#else
#define RAW_ASSERT(expr, ...) \
    do { \
        if (false) { \
            (void)(expr); \
            (void)(0 __VA_OPT__(, __VA_ARGS__)); \
        } \
    } while (0)
#endif
