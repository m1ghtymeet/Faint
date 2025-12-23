#include "Material.h"
#include <Core/GlobalLocator.h>
#include <Debug/Instrumentor.h>
#include <Debug/Assertion.h>
#include <AssetManagment/TextureManager.h>
#include <FileSystem/FileSystem.h>
#include <Util/ImageTools.h>
#include <filesystem>
#include <algorithm>
#include <execution>

namespace Moon::Rendering {
	MaterialProperty DefaultValueFromType(GLenum type) {
		MaterialProperty prop;
		prop.singleUse = false;
		
		switch (type) {
		case GL_FLOAT:
			prop.value = 0.0f;
			break;
		case GL_INT:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			prop.value = 0;
			break;
		case GL_FLOAT_VEC2:
			prop.value = glm::vec2(0.0f);
			break;
		case GL_FLOAT_VEC3:
			prop.value = glm::vec3(0.0f);
			break;
		case GL_FLOAT_VEC4:
			prop.value = glm::vec4(0.0f);
			break;
		case GL_FLOAT_MAT4:
			prop.value = glm::mat4(1.0f);
			break;
		default:
			prop.value = std::monostate{};
			break;
		}

		return prop;
	}
}

Moon::Rendering::Material::Material(std::shared_ptr<Shader> p_shader) :
	m_shader(p_shader), type(MaterialType::PBR) {
	if (m_shader)
		UpdateCache();
}

Moon::Rendering::Material::Material(const Material& p_other) {
	CopyFrom(p_other);
}

Moon::Rendering::Material& Moon::Rendering::Material::operator=(const Material& other) {
	if (this != &other)
		CopyFrom(other);
	return *this;
}

Moon::Rendering::Material::Material(Material&& other) noexcept {
	std::unique_lock lock1(m_propertiesMutex, std::defer_lock);
	std::unique_lock lock2(other.m_propertiesMutex, std::defer_lock);
	std::lock(lock1, lock2);

	m_shader = std::move(other.m_shader);
	m_properties = std::move(other.m_properties);
	m_gpuCache = std::move(other.m_gpuCache);
	m_textureSlotCache = std::move(other.m_textureSlotCache);
	m_systemTextures = std::move(other.m_systemTextures);

	type = other.type;
	path = std::move(other.path);

	m_blendMode = other.m_blendMode;
	m_blendable = other.m_blendable;
	m_backfaceCulling = other.m_backfaceCulling;
	m_frontfaceCulling = other.m_frontfaceCulling;
	m_depthTest = other.m_depthTest;
	m_depthWriting = other.m_depthWriting;
	m_colorWriting = other.m_colorWriting;
	m_castShadows = other.m_castShadows;
	m_receiveShadows = other.m_receiveShadows;
	m_instancingEnabled = other.m_instancingEnabled;
	m_maxInstances = other.m_maxInstances;
	m_renderQueue = other.m_renderQueue;

	m_isDirty.store(other.m_isDirty.load());
}

Moon::Rendering::Material& Moon::Rendering::Material::operator=(Material&& other) noexcept {
	if (this != &other) {
		std::unique_lock lock1(m_propertiesMutex, std::defer_lock);
		std::unique_lock lock2(other.m_propertiesMutex, std::defer_lock);
		std::lock(lock1, lock2);

		m_shader = std::move(other.m_shader);
		m_properties = std::move(other.m_properties);
		m_gpuCache = std::move(other.m_gpuCache);
		m_textureSlotCache = std::move(other.m_textureSlotCache);
		m_systemTextures = std::move(other.m_systemTextures);

		type = other.type;
		path = std::move(other.path);

		m_blendMode = other.m_blendMode;
		m_blendable = other.m_blendable;
		m_backfaceCulling = other.m_backfaceCulling;
		m_frontfaceCulling = other.m_frontfaceCulling;
		m_depthTest = other.m_depthTest;
		m_depthWriting = other.m_depthWriting;
		m_colorWriting = other.m_colorWriting;
		m_castShadows = other.m_castShadows;
		m_receiveShadows = other.m_receiveShadows;
		m_instancingEnabled = other.m_instancingEnabled;
		m_maxInstances = other.m_maxInstances;
		m_renderQueue = other.m_renderQueue;

		m_isDirty.store(other.m_isDirty.load());
	}
	return *this;
}

void Moon::Rendering::Material::MakeDirty() {
	m_isDirty.store(true, std::memory_order_release);
}

void Moon::Rendering::Material::MakeClean() {
	m_isDirty.store(false, std::memory_order_release);
}

bool Moon::Rendering::Material::IsDirty() const {
	return m_isDirty.load(std::memory_order_acquire);
}

void Moon::Rendering::Material::SetShader(std::shared_ptr<Moon::Shader> p_shader) {
	if (m_shader == p_shader) return;
	m_shader = p_shader;

	if (m_shader) {
		m_gpuCache.Clear();
		m_textureSlotCache.clear();
		MakeDirty();
	}
	else {
		std::unique_lock lock(m_propertiesMutex);
		m_properties.clear();
	}
	MakeDirty();
}

std::shared_ptr<Moon::Shader> Moon::Rendering::Material::GetShader() const {
	return m_shader;
}

void Moon::Rendering::Material::Bind(Rendering::Texture* p_emptyTexture2D) {
	FT_PROFILE_FUNCTION();

	FT_CORE_ASSERT(IsValid(), "Attempting to bind an invalid material.");

	if (m_properties.empty()) {
		HZ_CORE_WARN("Material has no properties to bind");
		return;
	}

	if (IsDirty()) {
		UpdateCache();
		MakeClean();
	}

	//ApplyBlendMode();
	BindUniforms();
	BindTextures();
}

void Moon::Rendering::Material::BindUniforms() {
	FT_PROFILE_FUNCTION();

	std::shared_lock lock(m_propertiesMutex);

	try {
		for (auto& [name, prop] : m_properties) {
			if (std::holds_alternative<Assets::Texture*>(prop.value)) {
				continue;
			}

			std::visit([&](auto&& value) {
				using T = std::decay_t<decltype(value)>;

				if constexpr (std::is_same_v<T, float>) m_shader->SetFloat(name, value);
				else if constexpr (std::is_same_v<T, int>) m_shader->SetInt(name, value);
				else if constexpr (std::is_same_v<T, bool>) m_shader->SetBool(name, value);
				else if constexpr (std::is_same_v<T, glm::vec2>) { m_shader->SetVec2(name, value); }
				else if constexpr (std::is_same_v<T, glm::vec3>) { m_shader->SetVec3(name, value); }
				else if constexpr (std::is_same_v<T, glm::mat4>) m_shader->SetMat4(name, value);
				}, prop.value);
		}
	}
	catch (const std::exception& e) {
		HZ_CORE_ERROR("Exception in Material::BindUniforms(): {}", e.what());
	}
}

void Moon::Rendering::Material::BindTextures() {
	FT_PROFILE_FUNCTION();

	std::shared_lock lock(m_propertiesMutex);

	try {
		for (const auto& [name, slot] : m_textureSlotCache) {
			if (auto prop = GetProperty(name)) {
				if (auto* texture = std::get_if<Assets::Texture*>(&prop->value)) {
					if (texture && *texture) {
						(*texture)->Bind(slot);
						m_shader->SetInt(name, slot);
					}
				}
			}
		}

		int baseSlot = static_cast<int>(m_textureSlotCache.size());
		for (const auto& [name, textureID] : m_systemTextures) {
			std::string uniformName = "u_" + name;
			if (m_shader->HasUniform(uniformName)) {
				glActiveTexture(GL_TEXTURE0 + baseSlot);
				glBindTexture(GL_TEXTURE_2D, textureID);
				m_shader->SetInt(uniformName, baseSlot);
				baseSlot++;
			}
		}
	}
	catch (const std::exception& e) {
		HZ_CORE_ERROR("Exception in Material::BindTextures(): {}", e.what());
	}
}

void Moon::Rendering::Material::Unbind() const {
	FT_CORE_ASSERT(IsValid(), "Attempting to unbind an invalid material.");
	//if (m_shader)
	//	m_shader->Unbind();
}

bool Moon::Rendering::Material::IsValid() const {
	return m_shader != nullptr;
}

void Moon::Rendering::Material::SetProperty(const std::string p_name, const MaterialPropertyType& p_value, bool p_singleUse) {
	FT_CORE_ASSERT(IsValid(), "Attempting to SetProperty on an invalid material.");
	//m_properties[p_name] = MaterialProperty{
	//	p_value,
	//	p_singleUse
	//};
	//MakeDirty();
	ModifyProperties([&](PropertyMap& props) {
		props[p_name] = MaterialProperty{ p_value, p_singleUse };
		});
}

void Moon::Rendering::Material::RemoveProperty(const std::string& name) {
	ModifyProperties([&](PropertyMap& props) {
		props.erase(name);
		});
}

std::optional<Moon::Rendering::MaterialProperty> Moon::Rendering::Material::GetProperty(const std::string p_key) const {
	return AccessProperties([&](const PropertyMap& props) -> std::optional<MaterialProperty> {
		auto it = props.find(p_key);
		if (it != props.end()) {
			return it->second;
		}
		return std::nullopt;
		});
}

Moon::Rendering::Material::PropertyMap& Moon::Rendering::Material::GetProperties() {
	return m_properties;
}

void Moon::Rendering::Material::SetBlendMode(BlendMode mode) {
	m_blendMode = mode;
}

Moon::Rendering::BlendMode Moon::Rendering::Material::GetBlendMode() const {
	return m_blendMode;
}

void Moon::Rendering::Material::SetBlendable(bool p_blended) {
	m_blendable = p_blended;
}

void Moon::Rendering::Material::SetBackfaceCulling(bool p_backface) {
	m_backfaceCulling = p_backface;
}

void Moon::Rendering::Material::SetFrontfaceCulling(bool p_frontface) {
	m_frontfaceCulling = p_frontface;
}

void Moon::Rendering::Material::SetDepthTest(bool p_depthTest) {
	m_depthTest = p_depthTest;
}

void Moon::Rendering::Material::SetDepthWriting(bool p_depthWrite) {
	m_depthWriting = p_depthWrite;
}

void Moon::Rendering::Material::SetColorWriting(bool p_colorWrite) {
	m_colorWriting = p_colorWrite;
}

bool Moon::Rendering::Material::IsBlendable() const {
	return m_blendable;
}

bool Moon::Rendering::Material::HasBackfaceCulling() const {
	return m_backfaceCulling;
}

bool Moon::Rendering::Material::HasDepthTest() const {
	return m_depthTest;
}

bool Moon::Rendering::Material::HasDepthWriting() const {
	return m_depthWriting;
}

bool Moon::Rendering::Material::HasColorWriting() const {
	return m_colorWriting;
}

json Moon::Rendering::Material::Serialize() {
	BEGIN_SERIALIZE();

	j["Type"] = static_cast<int>(type);
	j["BlendMode"] = static_cast<int>(m_blendMode);
	j["RenderQueue"] = m_renderQueue;

	if (m_shader)
		j["Shader"] = m_shader->GetPath();

	// Settings
	auto& settings = j["Settings"];
	settings["Blendable"] = m_blendable;
	settings["BackFaceCulling"] = m_backfaceCulling;
	settings["FrontFaceCulling"] = m_frontfaceCulling;
	settings["DepthTest"] = m_depthTest;
	settings["DepthWriting"] = m_depthWriting;
	settings["ColorWriting"] = m_colorWriting;
	settings["CastShadows"] = m_castShadows;
	settings["InstancingEnabled"] = m_instancingEnabled;
	settings["MaxInstances"] = m_maxInstances;

	// Uniforms
	std::shared_lock lock(m_propertiesMutex);

	for (auto& [name, prop] : m_properties) {
		std::visit([&](auto&& value) {
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<T, float>) {
				j["Uniforms"][name] = value;
			}
			else if constexpr (std::is_same_v<T, int>) {
				j["Uniforms"][name] = value;
			}
			else if constexpr (std::is_same_v<T, bool>) {
				j["Uniforms"][name] = value;
			}
			else if constexpr (std::is_same_v<T, glm::vec2>) {
				j["Uniforms"][name] = { value.x, value.y };
			}
			else if constexpr (std::is_same_v<T, glm::vec3>) {
				j["Uniforms"][name] = { value.x, value.y, value.z };
			}
			else if constexpr (std::is_same_v<T, Assets::Texture*>) {
				if (value != nullptr)
					j["Uniforms"][name] = FileSystem::AbsoluteToRelative(value->path);
			}
		}, prop.value);
	}

	END_SERIALIZE();
}

void Moon::Rendering::Material::Deserialize(const json& j) {

	if (j.contains("Type")) type = static_cast<MaterialType>(j["Type"].get<int>());
	if (j.contains("BlendMode")) SetBlendMode(static_cast<BlendMode>(j["BlendMode"].get<int>()));
	if (j.contains("RenderQueue")) m_renderQueue = j["RenderQueue"];

	if (j.contains("Shader")) {
		const std::string& sourcePath = j["Shader"].get<std::string>();
		bool absolute = std::filesystem::path(sourcePath).is_absolute();
		if (Moon::FileSystem::FileExists(sourcePath, !absolute)) {
			m_shader = std::make_shared<Shader>(std::vector<std::string>{ sourcePath });
		}
		else {
			m_shader = std::make_shared<Shader>(std::vector<std::string>{ "data/shaders/gBuffer.ftshader" });
		}
	}

	if (j.contains("Settings")) {
		json settings = j["Settings"];
		if (j.contains("Blendable"))		m_blendable = settings["Blendable"];
		if (j.contains("BackFaceCulling"))  m_backfaceCulling = settings["BackFaceCulling"];
		if (j.contains("FrontFaceCulling")) m_frontfaceCulling = settings["FrontFaceCulling"];
		if (j.contains("DepthTest"))		m_depthTest = settings["DepthTest"];
		if (j.contains("DepthWriting"))		m_depthWriting = settings["DepthWriting"];
		if (j.contains("ColorWriting"))		m_colorWriting = settings["ColorWriting"];
		if (j.contains("InstancingEnabled"))		m_instancingEnabled = settings["InstancingEnabled"];
		if (j.contains("MaxInstances"))		m_maxInstances = settings["MaxInstances"];
	}

	std::unique_lock lock(m_propertiesMutex);
	m_properties.clear();

	if (j.contains("Uniforms")) {
		for (auto& [key, val] : j["Uniforms"].items()) {
			if (val.is_object()) {
				if (val.contains("x") && val.contains("y")) {
					SetProperty(key, glm::vec2(val["x"].get<float>(), val["y"].get<float>()));
				}
				if (val.contains("x") && val.contains("y") && val.contains("z")) {
					SetProperty(key, glm::vec3(val["x"].get<float>(), val["y"].get<float>(), val["z"].get<float>()));
				}
			}
			else if (val.is_array() && val.size() == 2) {
				SetProperty(key, glm::vec2(val[0], val[1]));
			}
			else if (val.is_array() && val.size() == 3) {
				SetProperty(key, glm::vec3(val[0], val[1], val[2]));
			}
			else if (val.is_array() && val.size() == 4) {
				SetProperty(key, glm::vec4(val[0], val[1], val[2], val[3]));
			}
			else if (val.is_boolean()) {
				SetProperty(key, val.get<bool>());
			}
			else if (val.is_string()) {
				std::string texPath = FileSystem::RelativeToAbsolute(val.get<std::string>());
				if (!texPath.empty() && FileSystem::FileExists(texPath, true)) {
					SetTexture(key, texPath);
				}
			}
			else if (val.is_number_float()) {
				SetProperty(key, val.get<float>());
			}
			else if (val.is_number_integer()) {
				SetProperty(key, val.get<int>());
			}
		}
	}
	MakeDirty();
}

void Moon::Rendering::Material::CopyFrom(const Material& other) {
	std::unique_lock lock1(m_propertiesMutex, std::defer_lock);
	std::shared_lock lock2(other.m_propertiesMutex, std::defer_lock);
	std::lock(lock1, lock2);

	m_shader = other.m_shader;
	m_properties = other.m_properties;
	m_systemTextures = other.m_systemTextures;

	type = other.type;
	path = other.path;

	m_blendMode = other.m_blendMode;
	m_blendable = other.m_blendable;
	m_backfaceCulling = other.m_backfaceCulling;
	m_frontfaceCulling = other.m_frontfaceCulling;
	m_depthTest = other.m_depthTest;
	m_depthWriting = other.m_depthWriting;
	m_colorWriting = other.m_colorWriting;
	m_castShadows = other.m_castShadows;
	m_receiveShadows = other.m_receiveShadows;
	m_instancingEnabled = other.m_instancingEnabled;
	m_maxInstances = other.m_maxInstances;
	m_renderQueue = other.m_renderQueue;

	MakeDirty();
}

void Moon::Rendering::Material::UpdateTextureSlotCache() {
	m_textureSlotCache.clear();
	std::shared_lock lock(m_propertiesMutex);
	int slot = 0;
	for (const auto& [name, prop] : m_properties) {
		if (std::holds_alternative<Assets::Texture*>(prop.value)) {
			m_textureSlotCache[name] = slot++;
		}
	}
}

void Moon::Rendering::Material::ApplyBlendMode() {
	if (m_blendable) {
		glEnable(GL_BLEND);
		switch (m_blendMode) {
		case BlendMode::_TRANSLUCENT:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BlendMode::ADDITIVE:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case BlendMode::MODULATE:
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			break;
		default:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	}
	else {
		glDisable(GL_BLEND);
	}
}

void Moon::Rendering::Material::SetTexture(const std::string& name, const std::string& filepath) {
	if (!IsValid()) {
		HZ_CORE_ERROR("Cannot set texture on invalid material");
		return;
	}

	std::string finalPath = filepath;
	if (std::filesystem::path(filepath).extension() != ".dds") {
		const std::string cacheDir = Moon::FileSystem::GetParentPath(path) + "/";
		const std::string ddsPath = cacheDir + std::filesystem::path(filepath).stem().string() + ".dds";

		if (!FileSystem::FileExists(ddsPath, true)) {
			ImageTools::CreateAndExportDDS(filepath, ddsPath, false);
		}
		finalPath = ddsPath;
	}
	//Moon::Assets::Texture* texture = FTSERVICE(AssetManagment::TextureManager).LoadResource(ddsPath);
	Moon::Assets::Texture* texture = Loaders::TextureLoader::Create(finalPath);
	SetProperty(name, texture);
}

void Moon::Rendering::Material::UpdateCache() {
	FT_PROFILE_FUNCTION();

	UpdateTextureSlotCache();

	m_gpuCache.Clear();

	//std::shared_lock lock(m_propertiesMutex);

	// Build uniform buffer data
	for (const auto& [name, prop] : m_properties) {
		if (!std::holds_alternative<Assets::Texture*>(prop.value)) {
			// Store offset for quick access
			m_gpuCache.uniformOffsets[name] = m_gpuCache.uniformBuffer.size();

			// Add data to buffer (simplified - expand based on needs)
			std::visit([&](auto&& value) {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, float>) {
					m_gpuCache.uniformBuffer.push_back(value);
				}
				// Add other types as needed
				}, prop.value);
		}
	}

	m_gpuCache.needsUpdate = false;
}