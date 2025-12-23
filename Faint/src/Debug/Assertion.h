#pragma once

#ifdef FT_DEBUG
	#if defined(FT_PLATFORM_WINDOWS)
		#define FT_DEBUGBREAK() __debugbreak()
	#elif defined(FT_PLATFORM_LINUX)
		#include <signal.h>
		#define HZ_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
		#define FT_ENABLE_ASSERTS
#else
	#define FT_DEBUGBREAK()
#endif

#include <Debug/Log.h>

#ifdef FT_ENABLE_ASSERTS
	#define FT_ASSERT_IMPL(type, condition, msg, ...) \
			if (!(condition)) { \
				HZ_CORE##type##ERROR("Assertion Failed: {}\n"); \
				FT_DEBUGBREAK(); \
			}

	#define FT_ASSERT_WITH_MSG(type, condition, ...) \
			FT_ASSERT_IMPL(type, condition, __VA_ARGS__)

	#define FT_ASSERT_NO_MSG(type, condition) \
			FT_ASSERT_IMPL(type, condition, "Assertion '{}' failed", #condition)

	#define FT_GET_MACRO_NAME(_1, _2, NAME, ...) NAME
	#define FT_ASSERT_SELECTOR(...) FT_GET_MACRO_NAME(__VA_ARGS__, FT_ASSERT_WITH_MSG, FT_ASSERT_NO_MSG)

	#define FT_ASSERT(...) FT_ASSERT_SELECTOR(__VA_ARGS__)(_, __VA_ARGS__)
	#define FT_CORE_ASSERT(...) FT_ASSERT_SELECTOR(__VA_ARGS__)(_, __VA_ARGS__)
#else
	#define FT_ASSERT(x, ...)
	#define FT_CORE_ASSERT(x, ...)
#endif