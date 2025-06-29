#include "Material.h"

#include <glad/glad.h>

namespace Faint {

	Ref<Texture> Material::m_DefaultAlbedo;
	Ref<Texture> Material::m_DefaultAO;
	Ref<Texture> Material::m_DefaultNormal;
	Ref<Texture> Material::m_DefaultRoughness;
	Ref<Texture> Material::m_DefaultMetalness;
	Ref<Texture> Material::m_DefaultDisplacement;

	Material::Material()
	{
		//if (m_DefaultAlbedo == nullptr)
		//	m_DefaultAlbedo = TextureManager::Get()->GetTexture("data/textures/Default.png");
		//if (m_DefaultNormal == nullptr)
		//	m_DefaultNormal = TextureManager::Get()->GetTexture("data/textures/Default_Normal.png");
		//if (m_DefaultMetalness == nullptr)
		//	m_DefaultMetalness = TextureManager::Get()->GetTexture("data/textures/Default_Metalness.png");
		//if (m_DefaultRoughness == nullptr)
		//	m_DefaultRoughness = TextureManager::Get()->GetTexture("data/textures/Default_Roughness.png");

		//m_Albedo = m_DefaultAlbedo;
		//m_Normal = m_DefaultNormal;
		//m_AO = m_DefaultNormal;
		//m_Metalness = m_DefaultMetalness;
		//m_Roughness = m_DefaultRoughness;

		//data.m_AlbedoColor = Vec3(1, 1, 1);
	}

	Material::Material(const std::string albedo) {

		m_Albedo = CreateRef<Texture>(albedo);
		m_Name = albedo;
	}
	Material::Material(const Vec3 albedoColor) {
		//data.m_AlbedoColor = Vec3{ albedoColor.r, albedoColor.g, albedoColor.b };

		m_Name = "default";
	}
	void Material::Bind(Shader* shader) {

		// Albedo
		if (m_Albedo != nullptr) {
			data.hasAlbedo = 1;
			//m_Albedo->Bind(0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_Albedo->GetID());
		}

		// Metalness
		if (m_Metalness != nullptr) {
			data.hasMetalness = 1;
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_Metalness->GetID());
		}

		// Roughness
		if (m_Roughness != nullptr) {
			data.hasRoughness = 1;
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, m_Roughness->GetID());
		}

		// AO
		//if (m_AO != nullptr) {
		//	data.u_HasAO = 1;
		//	m_Albedo->Bind(3);
		//}

		// Normal
		if (m_Normal != nullptr) {
			data.hasNormal = 1;
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, m_Normal->GetID());
		}
	}

	void Material::SetName(const std::string name) {
		m_Name = name;
	}

	std::string Material::GetName() {
		return m_Name;
	}
}

