#include "LightComponent.h"

namespace Faint {

	LightComponent::LightComponent()
	{
		Direction = Vec3(0, 1, 0);
		Color = Vec3(1, 1, 1);
		Strength = 1.0f;
		Radius = 16.0f;
		SetCastShadows(true);

		for (int i = 0; i < 3; i++) {
			//Ref<Framebuffer2> shadowMap = CreateRef<Framebuffer2>(false, glm::vec2(4096, 4096));
			//Ref<Texture> texture = CreateRef<Texture>(glm::vec2(4096, 4096), GL_DEPTH_COMPONENT);
			//texture->SetParameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			//shadowMap->SetTexture(texture, GL_DEPTH_ATTACHMENT);
			//m_Framebuffers[i] = shadowMap;
		}
	}

	LightComponent::~LightComponent()
	{

	}

	void LightComponent::SetCastShadows(bool toggle)
	{
		CastShadows = toggle;
		if (CastShadows)
		{
			if (Type == LightType::Directional || Type == LightType::Spot || Type == LightType::Point)
			{
				for (int i = 0; i < CSM_AMOUNT; i++)
				{
					//m_Framebuffers[i] = CreateRef<Framebuffer2>(false, Vec2(4096, 4096));
					//auto texture = CreateRef<Texture>(Vec2(4096, 4096), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
					//texture->SetParameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
					//m_Framebuffers[i]->SetTexture(texture, GL_DEPTH_ATTACHMENT);
				}
			}
		}
		else {
			//for (int i = 0; i < CSM_AMOUNT; i++)
				//m_Framebuffers[i] = nullptr;
		}
	}
	Matrix4 LightComponent::GetProjection()
	{
		return glm::perspectiveFov(glm::radians(Outercutoff * 2.5f), 1.0f, 1.0f, 0.001f, 50.0f);
	}
}