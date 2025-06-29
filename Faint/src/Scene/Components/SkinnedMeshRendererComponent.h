#pragma once
#include "AComponent.h"
#include "AssetManagment/AssetManager.h"

namespace Faint {
    struct MeshRenderingEntry {
        std::string meshName;
        int materialIndex = 0;
        int emissiveColorTexutreIndex = -1;
        bool blendingEnabled = false;
        bool drawingEnabled = true;
        bool renderAsGlass = false;
        int meshIndex = -1;
        float furLength = 0.0f;
        float furShellDistanceAttenuation = 0.0f;
        float furUVScale = 0.0f;
    };

    struct JointWorldMatrix {
        const char* name;
        glm::mat4 worldMatrix;
    };

    struct AnimatedTransforms {
        std::vector<glm::mat4> local;
        std::vector<glm::mat4> worldspace;

        void Resize(int size) {
            local.resize(size);
            worldspace.resize(size);
        }

        const size_t GetSize() {
            return local.size();
        }
    };

	class SkinnedMeshRendererComponent : public AComponent {
    private:
        enum class AnimatedMode { BINDPOSE, ANIMATION, RAGDOLL };
    public:
        SkinnedMeshRendererComponent(Entity& p_owner);

        std::string GetName() override { return "SkinnedMeshRenderer"; }

        void Load(std::string skinnedModelPath);
        void PlayAnimation(const std::string& animationName, float speed);
        
        std::string ModelPath;
        Ref<SkinnedModel> Model;

        json Serialize() {
            BEGIN_SERIALIZE();

            END_SERIALIZE();
        }
        bool Deserialize(const json& j) {

        }
	};
}