#include "ShadersManager.h"

namespace Faint {

	std::map<std::string, Shader> ShaderManager::m_Shaders = std::map<std::string, Shader>();

	void ShaderManager::LoadShaders() {
		//m_Shaders["gBuffer"] = CreateScope<Shader>({ "data/shaders/gBuffer.vert", "data/shaders/gBuffer.frag" });
		//m_Shaders["deferred"] = Shader({ "data/shaders/deferred.vert", "data/shaders/deferred.frag" });
	}
	Shader ShaderManager::GetShader(const std::string& path, bool absolute) {
		std::string filepath = absolute ? path : "data/shaders/" + path;
		if (m_Shaders.find(path) == m_Shaders.end()) {
			m_Shaders[path] = Shader({ filepath + ".vert", filepath + ".frag" });
		}
		return m_Shaders[path];
	}
	void ShaderManager::RebuildShaders() {
		bool allSuccessed = true;
		for (auto& [_, shader] : m_Shaders) {
			if (!shader.Hotload()) {
				allSuccessed = false;
			}
		}
		if (allSuccessed) {
			HZ_CORE_TRACE("Hotloaded shaders");
		}
	}
}