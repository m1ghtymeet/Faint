#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Moon {
    struct ShaderSourceBlock {
        GLenum type;
        std::string source;
    };

    std::vector<ShaderSourceBlock> ParseShaderFile(const std::string& filepath);
    void ParseFile(const std::string& filepath, std::string& outputString, std::vector<std::string>& lineToFile, std::vector<std::string>& includedPaths);
    int GetErrorLineNumber(const std::string& error);
    std::string GetErrorMessage(const std::string& line);
    std::string GetLinkingErrors(unsigned int shader);
    std::string GetShaderCompileErrors(unsigned int shader, const std::string& filename);
    std::string GetShaderCompileErrors(unsigned int shader, const std::string& filename, const std::vector<std::string>& lineToFile);
    std::string StripBOM(const std::string& source);

    std::unordered_map<std::string, std::vector<float>> m_uniformCache;

    Shader::Shader(std::vector<std::string> shaderPaths) {
        m_shaderPaths = shaderPaths;
        Load(m_shaderPaths);
    }

    void Shader::Bind() {
        glUseProgram(m_handle);
    }

    void Shader::Unbind() {
        glUseProgram(0);
    }

    bool Shader::Load(std::vector<std::string> shaderPaths) {
        // Compile shader modules
        std::vector<ShaderModule> modules;
        for (std::string& shaderPath : shaderPaths) {
            auto blocks = ParseShaderFile(shaderPath);
            if (!blocks.empty()) {
                for (auto& block : blocks)
                    modules.emplace_back(block.type, block.source, shaderPath);
            }
            else
                modules.push_back(shaderPath);
        }
        // Print compilation errors
        bool errorsFound = false;
        for (ShaderModule& module : modules) {
            if (module.CompilationFailed()) {
                errorsFound = true;
                break;
            }
        }
        if (errorsFound) {
            std::cout << "\n-------------------------------------------------------------------------\n\n";
            for (ShaderModule& module : modules) {
                if (module.CompilationFailed()) {
                    std::cout << " COMPILATION ERROR: " << module.GetFilename() << "\n\n";
                    std::cout << module.GetErrors() << "\n";
                }
                glDeleteShader(module.GetHandle());
            }
            std::cout << "-------------------------------------------------------------------------\n";
            return false;
        }
        // Attempt to link
        int tempHandle = glCreateProgram();
        for (ShaderModule& module : modules) {
            glAttachShader(tempHandle, module.GetHandle());
        }
        glLinkProgram(tempHandle);
        std::string linkingErrors = GetLinkingErrors(tempHandle);

        // Print any errors
        if (linkingErrors.length()) {
            std::cout << "\n-------------------------------------------------------------------------\n\n";
            std::cout << " LINKING ERROR: ";
            for (int i = 0; i < modules.size(); i++) {
                std::cout << modules[i].GetFilename();
                if (i != modules.size() - 1) {
                    std::cout << "/";
                }
            }
            std::cout << linkingErrors << "\n";
            std::cout << "-------------------------------------------------------------------------\n";
            for (ShaderModule& module : modules) {
                glDeleteShader(module.GetHandle());
            }
            return false;
        }
        // Otherwise store the handle to the compiled shader
        else {
            if (m_handle != -1) {
                glDeleteProgram(m_handle);
            }
            m_handle = tempHandle;
            m_uniformLocations.clear();
        }
        for (ShaderModule& module : modules) {
            glDeleteShader(module.GetHandle());
        }
        return true;
    }

    bool Shader::Hotload() {
        return Load(m_shaderPaths);
    }

    void Shader::SetBool(const std::string& name, bool value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform1i(m_uniformLocations[name], (int)value);
    }

    void Shader::SetInt(const std::string& name, int value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform1i(m_uniformLocations[name], value);
    }

    void Shader::SetFloat(const std::string& name, float value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform1f(m_uniformLocations[name], value);
    }

    void Shader::SetMat2(const std::string& name, const glm::mat2& mat) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniformMatrix2fv(m_uniformLocations[name], 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::SetMat3(const std::string& name, const glm::mat3& mat) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniformMatrix3fv(m_uniformLocations[name], 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::SetMat4(const std::string& name, glm::mat4 value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniformMatrix4fv(m_uniformLocations[name], 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetUvec2(const std::string& name, const glm::uvec2& value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform2uiv(m_uniformLocations[name], 1, &value[0]);
    }

    void Shader::SetVec2(const std::string& name, const glm::vec2& value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform2fv(m_uniformLocations[name], 1, &value[0]);
    }

    void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
        //assert(this); // بررسی اینکه pointer معتبره
            //assert(m_handle != 0); // نباید shader handle صفر باشه

        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            GLint loc = glGetUniformLocation(m_handle, name.c_str());
            if (loc == -1) {
                std::cerr << "Warning: uniform '" << name << "' not found in shader " << m_handle << std::endl;
            }
            m_uniformLocations[name] = loc;
        }

        if (m_uniformLocations[name] != -1)
            glUniform3fv(m_uniformLocations[name], 1, &value[0]);
    }

    void Shader::SetVec4(const std::string& name, const glm::vec4& value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform4fv(m_uniformLocations[name], 1, &value[0]);
    }

    void Shader::SetVec2(const std::string& name, float x, float y) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform2f(m_uniformLocations[name], x, y);
    }

    void Shader::SetVec3(const std::string& name, float x, float y, float z) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform3f(m_uniformLocations[name], x, y, z);
    }

    void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform4f(m_uniformLocations[name], x, y, z, w);
    }

    void Shader::SetIVec2(const std::string& name, const glm::ivec2& value) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        // Use the integer uniform function
        glUniform2i(m_uniformLocations[name], value.x, value.y);
    }

    void Shader::SetIVec2Array(const std::string& name, const std::vector<glm::ivec2>& data) {
        if (m_uniformLocations.find(name) == m_uniformLocations.end()) {
            m_uniformLocations[name] = glGetUniformLocation(m_handle, name.c_str());
        }
        glUniform2iv(m_uniformLocations[name], static_cast<GLsizei>(data.size()), reinterpret_cast<const int*>(data.data()));
    }

    bool Shader::HasUniform(const std::string& name) const {
        if (m_handle == 0) return false;
        GLint location = glGetUniformLocation(m_handle, name.c_str());
        return location != -1;
    }

    int Shader::GetID() {
        return m_handle;
    }

    std::string Shader::GetPath() const {
		return m_shaderPaths.at(0); // Assuming the first path is the main shader file
    }

    std::vector<ShaderUniform> Shader::GetActiveUniforms() const {
        std::vector<ShaderUniform> uniforms;
        if (m_handle == 0) return uniforms;

        GLint count = 0;
        glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &count);
        GLint maxNameLen = 0;
        glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLen);
        std::vector<char> nameBuf(maxNameLen);

        for (GLint i = 0; i < count; i++) {
            GLsizei length = 0;
            GLint size = 0;
            GLenum type = 0;
            glGetActiveUniform(m_handle, (GLuint)i, maxNameLen, &length, &size, &type, nameBuf.data());
            std::string name(nameBuf.data(), length);

            if (!name.empty()) {
                size_t pos = name.find("[0]");
                if (pos != std::string::npos) name = name.substr(0, pos);
            }

            GLint location = glGetUniformLocation(m_handle, name.c_str());

            ShaderUniform u;
            u.name = name;
            u.glType = type;
            u.size = size;
            u.location = location;
            uniforms.push_back(u);
        }
        return uniforms;
    }

    ShaderModule::ShaderModule(const std::string& filename) {
        // Parse the source code
        std::vector<std::string> lineMap;
        std::vector<std::string> includedPaths;
        std::string prasedShaderSource = "";
        ParseFile(filename, prasedShaderSource, lineMap, includedPaths);

        // Strip any BOM characters. Apparently older drivers don't handle BOM bytes properly when compiling GLSL shaders
        prasedShaderSource = StripBOM(prasedShaderSource);

        // Get type based on extension
        std::string extension = std::filesystem::path(filename).extension().string();
        static const std::unordered_map<std::string, int> shaderTypeMap = {
            {".vert", GL_VERTEX_SHADER},
            {".frag", GL_FRAGMENT_SHADER},
            {".geom", GL_GEOMETRY_SHADER},
            {".tesc", GL_TESS_CONTROL_SHADER},
            {".tese", GL_TESS_EVALUATION_SHADER},
            {".comp", GL_COMPUTE_SHADER}
        };
        int shaderType = shaderTypeMap.contains(extension) ? shaderTypeMap.at(extension) : GL_NONE;

        // Check for errors
        const char* shaderCode = prasedShaderSource.c_str();
        m_handle = glCreateShader(shaderType);
        glShaderSource(m_handle, 1, &shaderCode, NULL);
        glCompileShader(m_handle);
        m_errors = GetShaderCompileErrors(m_handle, filename, lineMap);
        m_filename = filename;
    }

    ShaderModule::ShaderModule(unsigned int type, const std::string& source, const std::string& filename) {
        m_handle = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(m_handle, 1, &src, nullptr);
        glCompileShader(m_handle);

        m_errors = GetShaderCompileErrors(m_handle, filename);
        m_filename = filename;
    }

    int ShaderModule::GetHandle() {
        return m_handle;
    }

    bool ShaderModule::CompilationFailed() {
        return m_errors.length();
    }

    std::string& ShaderModule::GetFilename() {
        return m_filename;
    }

    std::string& ShaderModule::GetErrors() {
        return m_errors;
    }

    std::vector<std::string>& ShaderModule::GetLineMap() {
        return m_lineMap;
    }

    std::vector<ShaderSourceBlock> ParseShaderFile(const std::string& filepath) {
        std::ifstream file(filepath);
        std::string line;
        std::vector<ShaderSourceBlock> blocks;

        unsigned int currentType = GL_NONE;
        std::stringstream currentSource;

        std::string baseDir = std::filesystem::path(filepath).parent_path().string();
        std::vector<std::string> includedPaths;

        auto processSourceWithIncludes = [&](const std::string& source) -> std::string {
            std::stringstream input(source);
            std::stringstream output;
            std::string l;
            bool versionInserted = false;
            int lineNumber = 0;
            std::string filename = std::filesystem::path(filepath).filename().string();

            while (std::getline(input, l)) {
                if (l.find("#include") != std::string::npos) {
                    size_t start = l.find("\"") + 1;
                    size_t end = l.find("\"", start);
                    std::string includeFile = l.substr(start, end - start);
                    std::string includePath = std::filesystem::weakly_canonical(baseDir + "/" + includeFile).string();

                    if (std::find(includedPaths.begin(), includedPaths.end(), includePath) == includedPaths.end()) {
                        includedPaths.push_back(includePath);
                        std::string includeOutput;
                        std::vector<std::string> dummyLineMap;
                        ParseFile(includePath, includeOutput, dummyLineMap, includedPaths);
                        output << includeOutput;
                    }
                }
                else {
                    output << l << "\n";
                    lineNumber++;

                    // درج define بعد از اولین #version
                    if (!versionInserted && l.rfind("#version", 0) == 0) {
                        output << "#define ENABLE_BINDLESS 0\n";
                        lineNumber++;
                        versionInserted = true;
                    }
                }
            }

            return output.str();
            };

        auto pushBlock = [&](unsigned int type, std::stringstream& ss) {
            if (type != GL_NONE && ss.str().size() > 0) {
                std::string processedSource = processSourceWithIncludes(ss.str());
                blocks.push_back({ type, processedSource });
            }
            ss.str("");
            ss.clear();
        };

        while (std::getline(file, line)) {
            if (line.find("#shader") != std::string::npos) {
                pushBlock(currentType, currentSource);

                if (line.find("vertex") != std::string::npos) currentType = GL_VERTEX_SHADER;
                else if (line.find("fragment") != std::string::npos) currentType = GL_FRAGMENT_SHADER;
                else if (line.find("geometry") != std::string::npos) currentType = GL_GEOMETRY_SHADER;
                else if (line.find("compute") != std::string::npos) currentType = GL_COMPUTE_SHADER;
                else if (line.find("tesc") != std::string::npos) currentType = GL_TESS_CONTROL_SHADER;
                else if (line.find("tese") != std::string::npos) currentType = GL_TESS_EVALUATION_SHADER;
            }
            else {
                if (currentType != GL_NONE)
                    currentSource << line << "\n";
            }
        }

        pushBlock(currentType, currentSource);
        return blocks;
    }

    void ParseFile(const std::string& filepath, std::string& outputString, std::vector<std::string>& lineToFile, std::vector<std::string>& includedPaths) {
        std::string baseDir = std::filesystem::path(filepath).parent_path().string();
        std::string filename = std::filesystem::path(filepath).filename().string();
        std::ifstream file(filepath);
        std::string line;
        int lineNumber = 0;
        bool versionInserted = false;
        while (std::getline(file, line)) {
            // Contains #shader
            //if (line.find("#shader") != std::string::npos) {
            //    if (line.find("vertex") != std::string::npos) {
            //
            //    }
            //    else if (line.find("fragment") != std::string::npos) {
            //
            //    }
            //}
            // Handle includes
            if (line.find("#include") != std::string::npos) {
                size_t start = line.find("\"") + 1;
                size_t end = line.find("\"", start);
                std::string includeFile = line.substr(start, end - start);
                std::string includePath = std::filesystem::weakly_canonical(baseDir + "/" + includeFile).string();
                // Check if the included file is already in includedPaths
                if (std::find(includedPaths.begin(), includedPaths.end(), includePath) == includedPaths.end()) {
                    includedPaths.push_back(includePath);
                    ParseFile(includePath, outputString, lineToFile, includedPaths);
                }
            }
            else {
                outputString += line + "\n";
                lineToFile.emplace_back(filename + " (line " + std::to_string(lineNumber++) + ")");

                // Insert the define after the first #version directive
                if (/*BackEnd::RenderDocFound() &&*/ !versionInserted && line.rfind("#version", 0) == 0) {
                    outputString += "#define ENABLE_BINDLESS 0\n";
                    lineToFile.emplace_back(filename + " (line " + std::to_string(lineNumber++) + ")");
                    versionInserted = true;
                }
            }
        }
    }

    std::string GetShaderCompileErrors(unsigned int shader, const std::string& filename, const std::vector<std::string>& lineToFile) {
        int success;
        char infoLog[1024];
        std::string result = "";
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            // Parse error log to extract line numbers
            std::stringstream logStream(infoLog);
            std::string line;
            while (std::getline(logStream, line)) {
                if ((line.substr(0, 7) == "ERROR: ")) {
                    int lineNumber = GetErrorLineNumber(line);
                    if (lineNumber >= 0 && lineNumber < lineToFile.size()) {
                        result += "  " + lineToFile[lineNumber] + ": " + GetErrorMessage(line) + "\n";
                    }
                }
            }
        }
        return result;
    }

    std::string GetLinkingErrors(unsigned int shader) {
        int success;
        char infoLog[1024];
        std::string result = "";
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::stringstream logStream(infoLog);
            std::string line;
            while (std::getline(logStream, line)) {
                result = "  " + line + "\n";
            }
            std::cout << infoLog << "\n";
        }
        return result;
    }

    std::string GetShaderCompileErrors(unsigned int shader, const std::string& filename) {
        int success;
        char infoLog[1024];
        std::string result = "";
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            result += filename + ": " + infoLog;
        }
        return result;
    }

    int GetErrorLineNumber(const std::string& error) {
        size_t firstColon = error.find(':');
        if (firstColon != std::string::npos) {
            size_t secondColon = error.find(':', firstColon + 1);
            if (secondColon != std::string::npos) {
                size_t thirdColon = error.find(':', secondColon + 1);
                if (thirdColon != std::string::npos) {
                    std::string lineNumberStr = error.substr(secondColon + 1, thirdColon - secondColon - 1);
                    return std::stoi(lineNumberStr);
                }
            }
        }
        return -1;
    }

    std::string GetErrorMessage(const std::string& line) {
        size_t firstColon = line.find(':');
        if (firstColon != std::string::npos) {
            size_t secondColon = line.find(':', firstColon + 1);
            if (secondColon != std::string::npos) {
                size_t thirdColon = line.find(':', secondColon + 1);
                if (thirdColon != std::string::npos) {
                    size_t messageStart = thirdColon + 2; // Skip the colon and space
                    if (messageStart < line.length()) {
                        return line.substr(messageStart);
                    }
                }
            }
        }
        return ""; // Return empty string if parsing fails
    }

    std::string StripBOM(const std::string& source) {
        const std::string bom = "\xEF\xBB\xBF";
        if (source.compare(0, bom.size(), bom) == 0)
            return source.substr(bom.size());
        return source;
    }
}