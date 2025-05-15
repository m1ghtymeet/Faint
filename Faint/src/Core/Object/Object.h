#pragma once

#include <entt.hpp>
#include <type_traits>

#define FT_HASHED_STATIC_STR(name) inline static constexpr entt::hashed_string name = entt::hashed_string(#name);

#define FT_ENUM_BITWISE_IMPL(enumClassName) \
    inline enumClassName operator|(enumClassName lhs, enumClassName rhs) \
    { \
        return static_cast<enumClassName>(ToUnderlying(lhs) | ToUnderlying(rhs)); \
    } \
 \
    inline enumClassName operator&(enumClassName lhs, enumClassName rhs) \
    { \
        return static_cast<enumClassName>(ToUnderlying(lhs) & ToUnderlying(rhs)); \
    }

namespace Hazel {

	struct HashedFnName {
		FT_HASHED_STATIC_STR(GetComponentName)
		FT_HASHED_STATIC_STR(AddToEntity)
		FT_HASHED_STATIC_STR(RemoveFromEntity)
		FT_HASHED_STATIC_STR(ActionName)
	};

	struct HashedName {
		FT_HASHED_STATIC_STR(DisplayName)
	};

	enum class ComponentTypeTrait : uint16_t {
		None = 0,
		// Exposes the component to be added via the inspector
		InspectorExposed = 1 << 0,
	};

	template<typename Enum>
	constexpr std::underlying_type<Enum> ToUnderlying(Enum e) {
		return static_cast<std::underlying_type_t<Enum>>(e);
	}

	//HZ_ENUM_BITWISE_IMPL(ComponentTypeTrait);
}

#define FAINTCOMPONENT(klass, componentName)										\
public:																				\
	static std::string ClassName() {												\
		static std::string className = #klass;										\
	}																				\
																					\
	inline static auto ComponentFactory = entt::meta<klass>();						\
	static void InitializeComponentClass() {										\
		static bool initialized = false;											\
		if (initialized) return;													\
																					\
		ComponentFactory.type(entt::hashed_string(#klass))							\
			.traits(ComponentTypeTrait::InspectorExposed);							\
		initialized = true;															\
	}																				\
																					\
	template<auto Data>																\
	static auto BindComponentField(const char* varName, const char* displayName) {  \
			return ComponentFactory													\
				.data<Data>(entt::hashed_string(varName))							\
				.prop(HashedName::DisplayName, displayName);						\
		}