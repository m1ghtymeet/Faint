#pragma once
#include "hzpch.h"

// Platform detection using predefined macros
//#ifdef _WIN32
//		/* Windows x64/x86 */
//	#ifdef _WIN64
//		/* Windows x64 */
//		#define HZ_PLATFORM_WINDOWS
//	#else
//		#error "x86 Builds are not supported!"
//	#endif
//#elif defined(__APPLE__) || defined(__MACH__)
//	#if TARGET_IPHONE_SIMULATOR == 1
//		#error "IOS simulator is not supported!"
//	#endif
//#elif defined(__ANDROID__)
//	#define HZ_PLATFORM_ANDROID
//	#error "Android is not supported!"
//#elif defined(__linux__)
//	#define HZ_PLATFORM_LINUX
//	#error "Linux is not supported!"
//#else
//	/* Unknown compiler/platform */
//	#error "Unknown platform!"
//#endif // End of platform detection
//
//
#ifdef FT_PLATFORM_WINDOWS
	#if FT_DYNAMIC_LINK
		#ifdef FT_BUILD_DLL
			#define FAINT_API __declspec(dllexport)
		#else
			#define FAINT_API __declspec(dllimport)
		#endif
	#else
		#define HAZEL_API
	#endif
#else
	#error FAINT only supports Windows!
#endif

#include <memory>

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

#define HZ_EXPAND_MACRO(x) x
#define HZ_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#include <filesystem>

#ifdef FT_ENABLE_ASSERTS
	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define HZ_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { HZ##type##ERROR(msg, __VA_ARGS__); __debugbreak(); } }
	#define HZ_INTERNAL_ASSERT_WITH_MSG(type, check, ...) HZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: ", __VA_ARGS__)
	#define HZ_INTERNAL_ASSERT_NO_MSG(type, check) HZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", HZ_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)
	
	#define HZ_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define HZ_INTERNAL_ASSERT_GET_MACRO(...) HZ_EXPAND_MACRO( HZ_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, HZ_INTERNAL_ASSERT_WITH_MSG, HZ_INTERNAL_ASSERT_NO_MSG) )
	
	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define HZ_ASSERT(...) HZ_EXPAND_MACRO( HZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define HZ_CORE_ASSERT(...) HZ_EXPAND_MACRO( HZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define HZ_ASSERT(x, ...)
	#define HZ_CORE_ASSERT(x, ...)
#endif

#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <stdexcept>
#include <cassert>

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T>
using Weak = std::weak_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}