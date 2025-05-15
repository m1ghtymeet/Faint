#include "Material.h"

#include "Renderer/Manager/TextureManager.h"

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
		if (m_DefaultAlbedo == nullptr)
			m_DefaultAlbedo = TextureManager::Get()->GetTexture("data/textures/Default.png");
		if (m_DefaultNormal == nullptr)
			m_DefaultNormal = TextureManager::Get()->GetTexture("data/textures/Default_Normal.png");
		if (m_DefaultMetalness == nullptr)
			m_DefaultMetalness = TextureManager::Get()->GetTexture("data/textures/Default_Metalness.png");
		if (m_DefaultRoughness == nullptr)
			m_DefaultRoughness = TextureManager::Get()->GetTexture("data/textures/Default_Roughness.png");

		m_Albedo = m_DefaultAlbedo;
		m_Normal = m_DefaultNormal;
		m_AO = m_DefaultNormal;
		m_Metalness = m_DefaultMetalness;
		m_Roughness = m_DefaultRoughness;

		data.m_AlbedoColor = Vec3(1, 1, 1);

		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOStructure), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	Material::Material(const std::string albedo) {

		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOStructure), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		m_Albedo = TextureManager::Get()->GetTexture(albedo);

		m_Name = albedo;
	}
	Material::Material(const Vec3 albedoColor) {

		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOStructure), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		data.m_AlbedoColor = Vec3{ albedoColor.r, albedoColor.g, albedoColor.b };

		m_Name = "default";
	}
	void Material::Bind(Shader* shader) {

		// Albedo
		if (m_Albedo != nullptr) {
			data.u_HasAlbedo = 1;
			m_Albedo->Bind(0);
		}

		// Metalness
		if (m_Metalness != nullptr) {
			data.u_HasMetalness = 1;
			m_Metalness->Bind(1);
		}

		// Roughness
		if (m_Roughness != nullptr) {
			data.u_HasRoughness = 1;
			m_Roughness->Bind(2);
		}

		// AO
		if (m_AO != nullptr) {
			data.u_HasAO = 1;
			m_Albedo->Bind(3);
		}

		// Normal
		if (m_Normal != nullptr) {
			data.u_HasNormal = 1;
			m_Normal->Bind(4);
		}

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOStructure), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 32, UBO);
	}

	void Material::SetName(const std::string name) {
		m_Name = name;
	}

	std::string Material::GetName() {
		return m_Name;
	}
}

