#pragma once

#include "Core/Base.h"
#include "Shader.h"

namespace Faint {

	class ShaderManager {
	private:
		static std::map<std::string, Shader> m_Shaders;

	public:
		static void LoadShaders();
		static Shader GetShader(const std::string& path, bool absolute = false);
		static void RebuildShaders();
	};
}