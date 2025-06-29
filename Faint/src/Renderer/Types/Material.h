#pragma once
#include "Core/Base.h"
#include "Math/Math.h"

#include "Renderer/Shader.h"

#include "Renderer/Types/Texture.h"
#include "FileSystem/FileSystem.h"
#include "AssetManagment/Serializable.h"
#include "AssetManagment/Resource.h"

namespace Faint {
	struct MaterialData {
		int hasAlbedo;
		float padding0;
		float padding1;
		float padding2;
		glm::vec3 albedoColor;
		int hasNormal;
		int hasMetalness;
		int hasRoughness;
	};

	class Material : ISerializable, public Resource {
	private:
		std::string m_Name;
	public:
		int id;

		Ref<Texture> m_Albedo;
		Ref<Texture> m_AO;
		Ref<Texture> m_Metalness;
		Ref<Texture> m_Roughness;
		Ref<Texture> m_Normal;

		MaterialData data;

		static Ref<Texture> m_DefaultAlbedo;
		static Ref<Texture> m_DefaultAO;
		static Ref<Texture> m_DefaultMetalness;
		static Ref<Texture> m_DefaultRoughness;
		static Ref<Texture> m_DefaultNormal;
		static Ref<Texture> m_DefaultDisplacement;

		Material();
		Material(const std::string albedo);
		Material(Ref<Texture> texture) { m_Albedo = texture; }
		Material(const Vec3 albedoColor);
		~Material() = default;

		void Bind(Shader* shader);

		void SetName(const std::string name);
		std::string GetName();

		bool HasAlbedo() { return m_Albedo != nullptr; }
		void SetAlbedo(const std::string path) { m_Albedo = CreateRef<Texture>(path); }
		void SetAlbedo(Ref<Texture> texture) { m_Albedo = texture; }

		bool HasNormal() { return m_Normal != nullptr; }
		void SetNormal(const std::string path) { m_Normal = CreateRef<Texture>(path); }
		void SetNormal(Ref<Texture> texture) { m_Normal = texture; }

		bool HasMetalness() { return m_Metalness != nullptr; }
		void SetMetalness(const std::string path) { m_Metalness = CreateRef<Texture>(path); }
		void SetMetalness(Ref<Texture> texture) { m_Metalness = texture; }

		bool HasRoughness() { return m_Roughness != nullptr; }
		void SetRoughness(const std::string path) { m_Roughness = CreateRef<Texture>(path); }
		void SetRoughness(Ref<Texture> texture) { m_Roughness = texture; }

		json Serialize() override {
			BEGIN_SERIALIZE();
			j["Path"] = Path;
			j["UUID"] = static_cast<uint64_t>(id);
			j["HasAlbedo"] = this->HasAlbedo();
			if (HasAlbedo()) {
				j["Albedo"] = this->m_Albedo->Serialize();
				//Vec3 Color = Vec3();
				//SERIALIZE_VEC3(Color);
			}

			j["HasNormal"] = this->HasNormal();
			if (HasNormal())
				j["Normal"] = this->m_Normal->Serialize();

			j["HasMetalness"] = this->HasMetalness();
			if (HasMetalness()) {
				j["Metalness"] = m_Metalness->Serialize();
				j["Metalness"]["Value"] = 0;
			}

			j["HasRoughness"] = this->HasRoughness();
			if (HasRoughness()) {
				j["Roughness"] = m_Roughness->Serialize();
				j["Roughness"]["Value"] = 0;
			}

			END_SERIALIZE();
		}

		void Deserialize(const json& j) override {

			if (j.contains("Albedo")) {
				const auto& texturePath = j["Albedo"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				Ref<Texture> albedoTexture = CreateRef<Texture>(absolutePath);
				SetAlbedo(albedoTexture);
			}
			if (j.contains("Normal")) {
				const auto& texturePath = j["Normal"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				Ref<Texture> texture = CreateRef<Texture>(absolutePath);
				SetNormal(texture);
			}
			if (j.contains("Roughness")) {
				const auto& texturePath = j["Roughness"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				Ref<Texture> roughnessTexture = CreateRef<Texture>(absolutePath);
				SetRoughness(roughnessTexture);
			}
			if (j.contains("Metalness")) {
				const auto& texturePath = j["Metalness"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				Ref<Texture> roughnessTexture = CreateRef<Texture>(absolutePath);
				SetMetalness(roughnessTexture);
			}
		}
	};
}