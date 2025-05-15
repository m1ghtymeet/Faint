#include "ShadersManager.h"

//#define LoadEmbeddedShader(file) \
//	m_Shaders[file##_path] = CreateScope<Shader>("assets/shaders/" + file##_path, std::string(reinterpret_cast<const char*>(file), reinterpret_cast<const char*>(file) + file##_len));

namespace Faint {

	std::map<std::string, Shader> ShaderManager::m_Shaders = std::map<std::string, Shader>();

	void ShaderManager::LoadShaders() {

		//m_Shaders["gBuffer"] = CreateScope<Shader>({ "data/shaders/gBuffer.vert", "data/shaders/gBuffer.frag" });
		//m_Shaders["deferred"] = CreateScope<Shader>({ "data/shaders/deferred.vert", "data/shaders/deferred.frag" });
	}
	Shader ShaderManager::GetShader(const std::string& path, bool absolute) {

		std::string filepath = absolute ? path : "data/shaders/" + path;
		if (m_Shaders.find(path) == m_Shaders.end()) {
			m_Shaders[path] = Shader({ filepath + ".vert", filepath + ".frag" });
		}
		return m_Shaders[path];
	}
	void ShaderManager::RebuildShaders() {

		for (auto& s : m_Shaders) {
			s.second.Hotload();
		}
	}
}