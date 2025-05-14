#include "Physics.h"
#include "Renderer/Renderer.h"

namespace Faint::Physics {

    void SubmitDebugLinesToRenderer(const DebugRenderMode& debugRenderMode) {
        PxScene* pxScene = GetPxScene();
        std::vector<PxRigidActor*> ignoreList;

        // Prepare
        PxU32 nbActors = pxScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
        if (nbActors) {
            std::vector<PxRigidActor*> actors(nbActors);
            pxScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
            for (PxRigidActor* actor : actors) {
                PxShape* shape;
                actor->getShapes(&shape, 1);
                actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
                for (PxRigidActor* ignoredActor : ignoreList) {
                    if (ignoredActor == actor) {
                        actor->setActorFlag(PxActorFlag::eVISUALIZATION, false);
                    }
                }
                if (debugRenderMode == DebugRenderMode::PHYSX_RAYCAST) {
                    if (shape->getQueryFilterData().word0 == RaycastGroup::RAYCAST_DISABLED) {
                        actor->setActorFlag(PxActorFlag::eVISUALIZATION, false);
                    }
                }
                else if (debugRenderMode == DebugRenderMode::PHYSX_COLLISION) {
                    if (shape->getQueryFilterData().word1 == CollisionGroup::NO_COLLISION) {
                        actor->setActorFlag(PxActorFlag::eVISUALIZATION, false);
                    }
                }
            }
        }
        const PxRenderBuffer& renderBuffer = pxScene->getRenderBuffer();
        int vertexCount = renderBuffer.getNbLines() * 2;

        static std::vector<Vertex> vertices;
        vertices.clear();
        vertices.resize(vertexCount);

        for (unsigned int i = 0; i < renderBuffer.getNbLines(); i++) {
            const PxDebugLine& pxLine = renderBuffer.getLines()[i];

            glm::vec4 color;
            switch (debugRenderMode) {
                case DebugRenderMode::PHYSX_ALL:        color = GREEN;      break;
                case DebugRenderMode::PHYSX_COLLISION:  color = LIGHT_BLUE; break;
                case DebugRenderMode::PHYSX_RAYCAST:    color = RED;        break;
                default: color = WHITE; break;
            }

            Renderer::DrawLine(Physics::PxVec3toGlmVec3(pxLine.pos0), Physics::PxVec3toGlmVec3(pxLine.pos1), color);
        }
    }
}