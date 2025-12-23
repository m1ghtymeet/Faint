#include "ShaderTemplateManager.h"
#include <Debug/Log.h>
#include <FileSystem/FileSystem.h>

#include <glad/glad.h>

namespace Moon::Editor {
	static std::string ReadFile(const std::string& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			HZ_CORE_ERROR("Failed to open file: {}", path);
			return "";
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	static GLuint CompileShader(GLenum type, const std::string& source, const std::string& shaderName) {
		GLuint shader = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
			HZ_CORE_ERROR("[Shader Compile Error] {}:\n{}", shaderName, infoLog);
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
}

Moon::Editor::ShaderTemplateManager::ShaderTemplateManager() {
	LoadTemplates();
}

Moon::Editor::ShaderTemplateManager::~ShaderTemplateManager() {
	CleanupShaders();
}

void Moon::Editor::ShaderTemplateManager::LoadTemplates(const std::string& folderPath) {
	CleanupShaders();
	templates.clear();
	templateNames.clear();

	try {
		for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
			if (!entry.is_directory()) continue;

			std::string templateDir = entry.path().string();
			std::string metaPath = templateDir + "/metadata.json";

			if (!FileSystem::FileExists(metaPath, true)) {
				HZ_CORE_WARN("Skipping template {}: metadata.json", templateDir);
				continue;
			}

			std::ifstream metaFile(metaPath);
			nlohmann::json metaJson;
			try {
				metaFile >> metaJson;
			}
			catch (const std::exception& e) {
				HZ_CORE_ERROR("Invalid metadata in {}: {}", templateDir, e.what());
				continue;
			}

			std::string name = metaJson.value("name", "Unknown");
			std::string desc = metaJson.value("description", "");

			std::string vGBuffer = templateDir + "/gbuffer.vert";
			std::string fGBuffer = templateDir + "/gbuffer.frag";
			std::string fLighting = templateDir + "/lighting.mnshader";

			if (!FileSystem::FileExists(vGBuffer, true) || !FileSystem::FileExists(fGBuffer, true) || !FileSystem::FileExists(fLighting, true)) {
				HZ_CORE_WARN("Template '{}' skipped: missing shader files", name);
				continue;
			}

			GLuint progGBuffer = CompileShaders(vGBuffer, fGBuffer);
			GLuint progLighting = CompileShaders(vGBuffer, fLighting);

			if (progGBuffer == 0 || progLighting == 0) {
				HZ_CORE_ERROR("Failed to compile shaders for template: {}", name);
				if (progGBuffer) glDeleteProgram(progGBuffer);
				if (progLighting) glDeleteProgram(progLighting);
				continue;
			}

			ShaderTemplate tmpl;
			tmpl.name = name;
			tmpl.programGBuffer = progGBuffer;
			tmpl.programLighting = progLighting;

			std::string key = name;
			templates[key] = std::move(tmpl);
			templateNames.push_back(key);

			HZ_CORE_INFO("Shader Template loaded: {}", name);
		}

		if (!templates.empty() && m_currentTemplate == nullptr)
			SwitchTemplate(templateNames[0]);

		HZ_CORE_INFO("Total Shader Templates loaded: {}", templates.size());
	}
	catch (const std::filesystem::filesystem_error& err) {

	}
}

void Moon::Editor::ShaderTemplateManager::SwitchTemplate(const std::string& templateName) {
	auto it = templates.find(templateName);
	if (it == templates.end()) {
		HZ_CORE_ERROR("Shader Template not found: {}", templateName);
		return;
	}
	m_currentTemplate = &it->second;
	HZ_CORE_INFO("Switched to Shader Template: {}", m_currentTemplate->name);
}

const std::vector<std::string>& Moon::Editor::ShaderTemplateManager::GetAvailableTemplates() const {
	return templateNames;
}

const Moon::Editor::ShaderTemplate* Moon::Editor::ShaderTemplateManager::GetCurrentTemplate() const {
	return m_currentTemplate;
}

uint32_t Moon::Editor::ShaderTemplateManager::CompileShaders(const std::string& vertexPath, const std::string& fragmentPath) {
	std::string vertSource = ReadFile(vertexPath);
	std::string fragSource = ReadFile(fragmentPath);
	if (vertSource.empty() || fragmentPath.empty())
		return 0;

	GLuint vert = CompileShader(GL_VERTEX_SHADER, vertSource, vertexPath);
	GLuint frag = CompileShader(GL_FRAGMENT_SHADER, fragSource, fragmentPath);
	if (vert == 0 || frag == 0) {
		if (vert) glDeleteShader(vert);
		if (frag) glDeleteShader(frag);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		HZ_CORE_ERROR("[Shader Link Error] {} + {}:\n{}", vertexPath, fragmentPath, infoLog);
		glDeleteProgram(program);
		program = 0;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
	return program;
}

void Moon::Editor::ShaderTemplateManager::CleanupShaders() {
	for (auto& [name, tmpl] : templates)
	{
		if (tmpl.programGBuffer)  glDeleteProgram(tmpl.programGBuffer);
		if (tmpl.programLighting) glDeleteProgram(tmpl.programLighting);
	}
	templates.clear();
	m_currentTemplate = nullptr;
}
