#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace Moon::Editor {
	struct ShaderTemplate {
		std::string name;
		std::string description;
		uint32_t programGBuffer;
		uint32_t programLighting;
	};

	class ShaderTemplateManager {
	public:
		ShaderTemplateManager();
		~ShaderTemplateManager();

		void LoadTemplates(const std::string& folderPath = "data/editor/shaders/");
		void SwitchTemplate(const std::string& templateName);

		const std::vector<std::string>& GetAvailableTemplates() const;
		const ShaderTemplate* GetCurrentTemplate() const;

	private:
		std::unordered_map<std::string, ShaderTemplate> templates;
		std::vector<std::string> templateNames;
		ShaderTemplate* m_currentTemplate = nullptr;

		uint32_t CompileShaders(const std::string& vertexPath, const std::string& fragmentPath);
		void CleanupShaders();
	};
}