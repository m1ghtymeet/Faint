#pragma once

#include <AssetManagment/Serializable.h>
#include <Renderer/Shader.h>
#include <Types/Renderer/Texture.h>

#include <variant>
#include <mutex>
#include <memory>
#include <atomic>
#include <shared_mutex>
#include <unordered_map>
#include <optional>

namespace Moon::Rendering {

	// Material property types with extended support
	using MaterialPropertyType = std::variant<
		std::monostate,
		bool,
		int,
		float,
		glm::vec2,
		glm::vec3,
		glm::vec4,
		glm::mat3,
		glm::mat4,
		Assets::Texture*
	>;

	struct MaterialProperty {
		MaterialPropertyType value;
		bool singleUse = false;
		bool isDirty = true;

		MaterialProperty() = default;
		MaterialProperty(const MaterialPropertyType& v, bool single = false)
			: value(v), singleUse(single), isDirty(true) { }
	};

	enum class MaterialType {
		STANDARD,
		PBR,
		UNLIT,
		CUSTOM
	};

	enum class BlendMode {
		_OPAQUE,
		MASKED,
		_TRANSLUCENT,
		ADDITIVE,
		MODULATE
	};

	struct MaterialGPUCache {
		std::vector<float> uniformBuffer;
		std::unordered_map<std::string, size_t> uniformOffsets;
		bool needsUpdate = true;

		void Clear() {
			uniformBuffer.clear();
			uniformOffsets.clear();
			needsUpdate = true;
		}
	};

	/**
	* A material is a combination of a shader and some settings
	*/
	class Material : public ISerializable {
	public:
		using PropertyMap = std::map<std::string, MaterialProperty>;

		Material(std::shared_ptr<Moon::Shader> p_shader = nullptr);
		Material(const Material& p_other);
		Material& operator=(const Material& other);
		Material(Material&& other) noexcept;
		Material& operator=(Material&& other) noexcept;
		~Material() = default;

		// Shader Managment
		void SetShader(std::shared_ptr<Moon::Shader> p_shader);
		std::shared_ptr<Moon::Shader> GetShader() const;
		bool IsValid() const;

		// Binding & Rendering
		void Bind(Rendering::Texture* p_emptyTexture2D = nullptr);
		void Unbind() const;
		void BindTextures();
		void BindUniforms();

		// Property Managment
		void SetProperty(const std::string p_name, const MaterialPropertyType& p_value, bool p_singleUse = false);
		void RemoveProperty(const std::string& name);
		std::optional<MaterialProperty> GetProperty(const std::string p_key) const;
		PropertyMap& GetProperties();

		// Material State
		void SetBlendMode(BlendMode mode);
		BlendMode GetBlendMode() const;

		void SetBlendable(bool p_blended);
		bool IsBlendable() const;

		void SetBackfaceCulling(bool p_backface);
		bool HasBackfaceCulling() const;

		void SetFrontfaceCulling(bool p_frontface);

		void SetDepthTest(bool p_depthTest);
		bool HasDepthTest() const;

		void SetDepthWriting(bool p_depthWrite);
		bool HasDepthWriting() const;

		void SetColorWriting(bool p_colorWrite);
		bool HasColorWriting() const;

		// Advanced Features
		//void SetRenderQueue(int queue);
		//int GetRenderQueue() const;

		// Texture Managment
		void SetTexture(const std::string& name, const std::string& filepath);
		//void SetTexture(const std::string& name, Assets::Texture* texture);
		//Assets::Texture* GetTexture(const std::string& name) const;

		void MakeDirty();
		void MakeClean();
		bool IsDirty() const;

		//std::string GetDebugInfo() const;

		virtual json Serialize() override;
		virtual void Deserialize(const json& j) override;

		std::string path;
		MaterialType type;
		std::unordered_map<std::string, uint32_t> m_systemTextures;

	private:
		void CopyFrom(const Material& other);
		void UpdateTextureSlotCache();
		void ApplyBlendMode();

		void UpdateCache();

	private:
		std::shared_ptr<Moon::Shader> m_shader;
		PropertyMap m_properties;
		mutable std::shared_mutex m_propertiesMutex;

		MaterialGPUCache m_gpuCache;
		std::unordered_map<std::string, int> m_textureSlotCache;
		std::atomic<bool> m_isDirty{ true };

		// Thread-Safe Property Access
		template<typename Func>
		auto AccessProperties(Func&& func) const -> decltype(func(m_properties));

		template<typename Func>
		void ModifyProperties(Func&& func);

		BlendMode m_blendMode = BlendMode::_OPAQUE;
		bool m_blendable = false;
		bool m_backfaceCulling = true;
		bool m_frontfaceCulling = false;
		bool m_depthTest = true;
		bool m_depthWriting = true;
		bool m_colorWriting = true;
		bool m_castShadows = true;
		bool m_receiveShadows = true;

		bool m_instancingEnabled = false;
		int m_maxInstances = 1;
		int m_renderQueue = 2000;
	};
}

#include "Material.inl"