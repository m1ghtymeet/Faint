#include "Material.h"

template<typename Func>
auto Moon::Rendering::Material::AccessProperties(Func&& func) const -> decltype(func(m_properties)) {
	//std::shared_lock lock(m_propertiesMutex);
	return func(m_properties);
}

template<typename Func>
void Moon::Rendering::Material::ModifyProperties(Func&& func) {
	//std::unique_lock lock(m_propertiesMutex);
	func(m_properties);
	MakeDirty();
}