#pragma once
#include "Core/Base.h"
#include "Math/Math.h"

#include "Renderer/Shader.h"
#include "Renderer/Manager/TextureManager.h"

#include "Renderer/Types/Texture.h"
#include "FileSystem/FileSystem.h"
#include "AssetManagment/Serializable.h"
#include "AssetManagment/Resource.h"

namespace Faint {
	
	class Material : ISerializable, public Resource {
		struct UBOStructure {
			int u_HasAlbedo;
			float padding;
			float padding2;
			float padding3;
			Vec3 m_AlbedoColor;
			int u_HasMetalness;
			float u_MetalnessValue;
			int u_HasRoughness;
			float u_RoughnessValue;
			int u_HasAO;
			float u_AOValue;
			int u_HasNormal;
			int u_HasDisplacement;
			int u_Unlit;
			float u_Emissive;
		};
	private:
		std::string m_Name;
		unsigned int UBO;

	public:
		Ref<Texture> m_Albedo;
		Ref<Texture> m_AO;
		Ref<Texture> m_Metalness;
		Ref<Texture> m_Roughness;
		Ref<Texture> m_Normal;

		UBOStructure data;

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
			}
			Vec3 albedoColor = data.m_AlbedoColor;
			SERIALIZE_VEC3(albedoColor);

			SERIALIZE_VAL_LBL("Emissive", data.u_Emissive);
			SERIALIZE_VAL_LBL("AOValue", data.u_AOValue);
			SERIALIZE_VAL_LBL("MetalnessValue", data.u_MetalnessValue);
			SERIALIZE_VAL_LBL("RoughnessValue", data.u_RoughnessValue);
			SERIALIZE_VAL_LBL("Unlit", data.u_Unlit);

			//j["HasAO"] = this->HasAO();
			
			j["HasMetalness"] = this->HasMetalness();
			if (HasMetalness())
				j["Roughness"] = m_Metalness->Serialize();

			j["HasRoughness"] = this->HasRoughness();
			if (HasRoughness())
				j["Roughness"] = m_Roughness->Serialize();

			END_SERIALIZE();
		}

		bool Deserialize(const json& j) override {

			if (j.contains("Albedo")) {
				const auto& texturePath = j["Albedo"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				Ref<Texture> albedoTexture = TextureManager::Get()->GetTexture(texturePath);
				SetAlbedo(albedoTexture);
			}
			if (j.contains("Roughness")) {
				const auto& texturePath = j["Roughness"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				Ref<Texture> roughnessTexture = TextureManager::Get()->GetTexture(texturePath);
				SetRoughness(roughnessTexture);
			}
			return true;
		}

		bool SerializeYaml(YAML::Emitter& emit) {

			emit << YAML::Key << "Material" << YAML::Value;
			emit << YAML::BeginMap;
			{
				emit << YAML::Key << "Path" << YAML::Value << Path;
				//emit << YAML::Key << "UUID" << YAML::Value << static_cast<uint64_t>(id);
				//emit << YAML::Key << "HasAlbedo" << YAML::Value << this->HasAlbedo();

				//if (HasAlbedo())
				//	m_Albedo->SerializeYaml();
				//Vec3 albedoColor = data.m_AlbedoColor;
				//emit << YAML::Key << "AlbedoColor" << YAML::Value << data.m_AlbedoColor;

				//emit << YAML::Key << "Emissive" << YAML::Value << data.u_Emissive;
				//emit << YAML::Key << "AOValue" << YAML::Value << data.u_AOValue;
				//emit << YAML::Key << "MetalnessValue" << YAML::Value << data.u_MetalnessValue;
			}
			emit << YAML::EndMap;
			return true;
		}
	};
}