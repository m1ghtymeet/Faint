#pragma once

#include "Component.h"
#include "AssetManagment/Serializable.h"

#include "Renderer/Light.h"
#include "Renderer/Types/Framebuffer2.h"

namespace Faint {

	const int CSM_AMOUNT = 4;
	
	class LightComponent : public Component {
		
		//HAZELCOMPONENT(BoxColliderComponent, "Box Collider");

		static void InitializeComponentClass() {
			//BindComponentField<&BoxColliderComponent::IsTrigger>("IsTrigger", "Is Trigger");
			//BindComponentFieldProperty<&BoxColliderComponent::SetSize>("IsTrigger", "Is Trigger");
		}
	public:
		LightType Type = Point;
		Vec3 Direction;
		float Cutoff = 12.0f;
		float Outercutoff = 20.0f;
		Vec3 Color;

		float Strength;
		float Radius;

		bool SyncDirectionWithSky = false;
		bool CastShadows = true;
		bool IsVolumetric = false;

        Matrix4 mViewProjections[CSM_AMOUNT];
        float mCascadeSplitDepth[CSM_AMOUNT];
        float mCascadeSplits[CSM_AMOUNT];

        float nearPlane = 0.1f, farPlane = 100.0f;
        float orthoSize = 20.0f;
        glm::mat4 lightSpaceMatrix;

		LightComponent();
		~LightComponent();

        void SetCastShadows(bool toggle);
        Matrix4 GetProjection();

        void CalculateViewProjection(glm::vec3 pos) {
            glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
            glm::mat4 lightView = glm::lookAt(
                pos,
                glm::vec3(0.0f),
                glm::vec3(0.0f, 1.0, 0.0f)
            );
            lightSpaceMatrix = lightProjection * lightView;
        }
		//void CalculateViewProjection(Matrix4& view, const Matrix4& projection)
		//{
        //    glm::mat4 viewProjection = projection * view;
        //    glm::mat4 inverseViewProjection = glm::inverse(viewProjection);
        //
        //    // TODO: Automate this
        //    const float nearClip = 0.01f;
        //    const float farClip = 800.0f;
        //    const float clipRange = farClip - nearClip;
        //
        //    const float mCascadeNearPlaneOffset = -100.0f;
        //    const float mCascadeFarPlaneOffset = 0.0;
        //
        //    // Calculate the optimal cascade distances
        //    const float minZ = nearClip;
        //    const float maxZ = nearClip + clipRange;
        //    const float range = maxZ - minZ;
        //    const float ratio = maxZ / minZ;
        //    for (int i = 0; i < CSM_AMOUNT; i++)
        //    {
        //        const float p = (i + 1) / static_cast<float>(4);
        //        const float log = minZ * glm::pow(ratio, p);
        //        const float uniform = minZ + range * p;
        //        const float d = 0.91f * (log - uniform) + uniform;
        //        mCascadeSplits[i] = (d - nearClip) / clipRange;
        //    }
        //
        //    mCascadeSplits[0] = 0.01f;
        //    //mCascadeSplits[1] = 0.45f;
        //    //mCascadeSplits[2] = 1.0f;
        //
        //    float lastSplitDist = 0.0f;
        //    // Calculate Orthographic Projection matrix for each cascade
        //    for (int cascade = 0; cascade < CSM_AMOUNT; cascade++)
        //    {
        //        float splitDist = mCascadeSplits[cascade];
        //        glm::vec4 frustumCorners[8] =
        //        {
        //            //Near face
        //            {  1.0f,  1.0f, -1.0f, 1.0f },
        //            { -1.0f,  1.0f, -1.0f, 1.0f },
        //            {  1.0f, -1.0f, -1.0f, 1.0f },
        //            { -1.0f, -1.0f, -1.0f, 1.0f },
        //
        //            //Far face
        //            {  1.0f,  1.0f, 1.0f, 1.0f },
        //            { -1.0f,  1.0f, 1.0f, 1.0f },
        //            {  1.0f, -1.0f, 1.0f, 1.0f },
        //            { -1.0f, -1.0f, 1.0f, 1.0f },
        //        };
        //
        //        // Project frustum corners into world space from clip space
        //        for (int i = 0; i < 8; i++)
        //        {
        //            glm::vec4 invCorner = inverseViewProjection * frustumCorners[i];
        //            frustumCorners[i] = invCorner / invCorner.w;
        //        }
        //        for (int i = 0; i < CSM_AMOUNT; i++)
        //        {
        //            glm::vec4 dist = frustumCorners[i + 4] - frustumCorners[i];
        //            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
        //            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        //        }
        //
        //        // Get frustum center
        //        glm::vec3 frustumCenter = glm::vec3(0.0f);
        //        for (int i = 0; i < 8; i++)
        //            frustumCenter += glm::vec3(frustumCorners[i]);
        //        frustumCenter /= 8.0f;
        //
        //        // Get the minimum and maximum extents
        //        float radius = 0.0f;
        //        for (int i = 0; i < 8; i++)
        //        {
        //            float distance = glm::length(glm::vec3(frustumCorners[i]) - frustumCenter);
        //            radius = glm::max(radius, distance);
        //        }
        //        radius = std::ceil(radius * 16.0f) / 16.0f;
        //        glm::vec3 maxExtents = glm::vec3(radius);
        //        glm::vec3 minExtents = -maxExtents;
        //
        //        // Calculate the view and projection matrix
        //        glm::vec3 lightDir = -this->Direction;
        //        glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 0.0f, 1.0f));
        //        glm::mat4 lightProjectionMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + mCascadeNearPlaneOffset, maxExtents.z - minExtents.z + mCascadeFarPlaneOffset);
        //
        //        // Offset to texel space to avoid shimmering ->(https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
        //        glm::mat4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
        //        const float ShadowMapResolution = 4096;
        //        glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
        //        glm::vec4 roundedOrigin = glm::round(shadowOrigin);
        //        glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
        //        roundOffset = roundOffset * 2.0f / ShadowMapResolution;
        //        roundOffset.z = 0.0f;
        //        roundOffset.w = 0.0f;
        //        lightProjectionMatrix[3] += roundOffset;
        //
        //        // Store SplitDistance and ViewProjection-Matrix
        //        mCascadeSplitDepth[cascade] = (nearClip + splitDist * clipRange) * 1.0f;
        //        mViewProjections[cascade] = shadowMatrix;
        //        lastSplitDist = mCascadeSplits[cascade];
        //    }
		//}

		json Serialize() {
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Type);
			SERIALIZE_VAL(Strength);
			SERIALIZE_VAL(SyncDirectionWithSky);
			SERIALIZE_VAL(CastShadows);
			SERIALIZE_VAL(Cutoff);
			SERIALIZE_VAL(Outercutoff);
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			if (j.contains("Type"))
				Type = (LightType)j["Type"];
			if (j.contains("IsVolumetric"))
				IsVolumetric = j["IsVolumetric"];
			if (j.contains("Color"))
				Color = Vec3(j["Color"]["x"], j["Color"]["y"], j["Color"]["z"]);
			if (j.contains("Strength"))
				Strength = j["Strength"];
			if (j.contains("SyncDirectionWithSky"))
				SyncDirectionWithSky = j["SyncDirectionWithSky"];
			if (j.contains("CastShadows"))
                SetCastShadows(j["CastShadows"]);
			if (j.contains("Direction"))
			{
				float x = j["Direction"]["x"];
				float y = j["Direction"]["y"];
				float z = j["Direction"]["z"];
				this->Direction = Vec3(x, y, z);
			}
			return true;
		}
	};
}