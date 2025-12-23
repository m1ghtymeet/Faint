#include "AssetManager.h"

namespace Moon::AssetManagment {

	template<typename T>
	inline T* AAssetManager<T>::LoadResource(const std::string& p_path) {
		if (auto resource = GetResource(p_path, false); resource)
			return resource;
		else {
			auto newResource = CreateResource(p_path);
			if (newResource)
				return RegisterResource(p_path, newResource);
			else
				return nullptr;
		}
	}

	template<typename T>
	inline void AAssetManager<T>::UnloadResource(const std::string& p_path) {
		if (auto resource = GetResource(p_path, false); resource) {
			DestroyResource(resource);
			UnregisterResource(p_path);
		}
	}

	template<typename T>
	inline bool AAssetManager<T>::IsResourceRegistered(const std::string& p_path) {
		return m_resources.find(p_path) != m_resources.end();
	}

	template<typename T>
	inline T* AAssetManager<T>::RegisterResource(const std::string& p_path, T* p_instance) {
		if (auto resource = GetResource(p_path, false); resource)
			DestroyResource(resource);

		m_resources[p_path] = p_instance;
		return p_instance;
	}

	template<typename T>
	inline void AAssetManager<T>::UnregisterResource(const std::string& p_path) {
		m_resources.erase(p_path);
	}

	template<typename T>
	inline T* AAssetManager<T>::GetResource(const std::string& p_path, bool p_tryToLoadIfNotFound) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (auto resource = m_resources.find(p_path); resource != m_resources.end()) {
			return resource->second;
		}
		else if (p_tryToLoadIfNotFound) {
			return LoadResource(p_path);
		}
		return nullptr;
	}

	template<typename T>
	inline T* AAssetManager<T>::operator[](const std::string& p_path) {
		return GetResource(p_path);
	}

	template<typename T>
	inline std::unordered_map<std::string, T*>& AAssetManager<T>::GetResources() {
		return m_resources;
	}
}