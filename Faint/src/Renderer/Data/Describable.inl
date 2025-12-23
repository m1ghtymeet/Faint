#include "Describable.h"
#include <memory>
#include <Debug/Assertion.h>

template<typename T>
inline void Moon::Rendering::Data::Describable::AddDescriptor(T&& p_descriptor) {
	FT_CORE_ASSERT(!HasDescriptor<T>(), "Descriptor already exists");
	auto ptr = std::make_shared<std::decay_t<T>>(std::forward<T>(p_descriptor));
	m_descriptors.emplace(typeid(std::decay_t<T>), ptr);
}

template<typename T>
inline void Moon::Rendering::Data::Describable::RemoveDescriptor() {
	FT_CORE_ASSERT(HasDescriptor<T>(), "Descriptor doesn't exist.");
	if (auto it = m_descriptors.find(typeid(T)); it != m_descriptors.end())
		m_descriptors.erase(it);
}

template<typename T>
inline bool Moon::Rendering::Data::Describable::HasDescriptor() const {
	return m_descriptors.contains(typeid(T));
}

template<typename T>
inline T& Moon::Rendering::Data::Describable::GetDescriptor() {
	auto it = m_descriptors.find(typeid(T));
	FT_CORE_ASSERT(it != m_descriptors.end(), "Couldn't find a descriptor matching the given type T.");
	auto ptr = std::any_cast<std::shared_ptr<T>>(it->second);
	FT_CORE_ASSERT(ptr, "Descriptor pointer is null");
	return *ptr;
}
