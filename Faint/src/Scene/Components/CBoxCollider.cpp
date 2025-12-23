#include "CBoxCollider.h"
#include <AssetManagment/MeshManager.h>
#include <Scene/Entity.h>
#include <Scene/Components/CModelRenderer.h>
#include <Physics/Types/PhysicsBox.h>
#include <Physics/Physics.h>

glm::vec3 CalculateEntityAABB(Moon::Entity& entity, glm::vec3& outMin, glm::vec3& outMax) {
	bool first = true;
	glm::vec3 globalMin, globalMax;

	auto updateAABB = [&](const Moon::MeshRendererComponent& meshComp) {
		Moon::Model* model = meshComp.GetModel();
		if (!model) return;

		for (auto meshIndex : model->GetMeshIndices()) {
			const auto& mesh = Moon::AssetManagment::MeshManager::GetMeshByIndex(meshIndex);
			glm::mat4 world = entity.transform->GetGlobalMatrix();

			glm::vec3 corners[8] = {
				{mesh->aabbMin.x, mesh->aabbMin.y, mesh->aabbMin.z},
				{mesh->aabbMax.x, mesh->aabbMin.y, mesh->aabbMin.z},
				{mesh->aabbMin.x, mesh->aabbMax.y, mesh->aabbMin.z},
				{mesh->aabbMax.x, mesh->aabbMax.y, mesh->aabbMin.z},
				{mesh->aabbMin.x, mesh->aabbMin.y, mesh->aabbMax.z},
				{mesh->aabbMax.x, mesh->aabbMin.y, mesh->aabbMax.z},
				{mesh->aabbMin.x, mesh->aabbMax.y, mesh->aabbMax.z},
				{mesh->aabbMax.x, mesh->aabbMax.y, mesh->aabbMax.z}
			};

			for (int i = 0; i < 8; i++) {
				glm::vec3 worldPos = glm::vec3(world * glm::vec4(corners[i], 1.0f));
				if (first) {
					globalMin = globalMax = worldPos;
					first = false;
				}
				else {
					globalMin = glm::min(globalMin, worldPos);
					globalMax = glm::max(globalMax, worldPos);
				}
			}
		}
		};

	updateAABB(*entity.GetComponent<Moon::MeshRendererComponent>());

	outMin = globalMin;
	outMax = globalMax;

	return globalMax - globalMin; // size
}

Moon::BoxColliderComponent::BoxColliderComponent(Entity& p_owner)
	: RigidBodyComponent(p_owner) {

	m_physicsActor = std::make_unique<Physics::PhysicsBox>(
		owner.transform->GetTransform(),
		halfExtents,
		m_mass
	);
	m_physicsActor->SetUserData<std::reference_wrapper<ColliderComponent>>(*this);
	m_physicsActor->CreateActor();
	BindListener();
	Init();

	if (auto* mesh = owner.GetComponent<MeshRendererComponent>()) {
		glm::vec3 min, max;
		glm::vec3 size = CalculateEntityAABB(owner, min, max);
		glm::vec3 center = (min + max) * 0.5f;
		glm::vec3 localCenter = center - owner.transform->GetGlobalPosition();
		SetSize(size * 0.5f);
		//glm::quat identityRot = glm::identity<glm::quat>();
		//SetShapeOffset(localCenter, identityRot);
	}
}

std::string Moon::BoxColliderComponent::GetName() {
	return "BoxCollider";
}

void Moon::BoxColliderComponent::SetTrigger(bool trigger) {
	if (m_physicsActor)
		m_physicsActor->SetTrigger(trigger);
	isTrigger = trigger;
}

void Moon::BoxColliderComponent::SetSize(const glm::vec3& newSize) {
	if (halfExtents != newSize) {
		halfExtents = newSize;
		GetPhysicsActorAs<Physics::PhysicsBox>().Reshape(newSize, shapeOffset);	
	}
}

void Moon::BoxColliderComponent::SetShapeOffset(const glm::vec3& posOffset, const glm::quat& rotOffset) {
	shapeOffset = posOffset;
	GetPhysicsActorAs<Physics::PhysicsBox>().Reshape(halfExtents, posOffset, rotOffset);
}

glm::vec3 Moon::BoxColliderComponent::GetSize() {
	return halfExtents;
}

void Moon::BoxColliderComponent::Init() {
	m_physicsActor->m_entity = &owner;
	m_physicsActor->m_name = owner.GetTag();
}

void Moon::BoxColliderComponent::OnEnable() {
	if (m_physicsActor) {
		m_physicsActor->SetEnabled(true);
	}
}

void Moon::BoxColliderComponent::OnDisable() {
	if (m_physicsActor) {
		m_physicsActor->SetEnabled(false);
	}
}

void Moon::BoxColliderComponent::OnUpdate(float p_deltaTime) {
	
}

json Moon::BoxColliderComponent::Serialize() {
	BEGIN_SERIALIZE();
	{
		j["halfExtents"]["x"] = halfExtents.x;
		j["halfExtents"]["y"] = halfExtents.y;
		j["halfExtents"]["z"] = halfExtents.z;
	}
	{
		j["shapePosOffset"]["x"] = shapeOffset.x;
		j["shapePosOffset"]["y"] = shapeOffset.y;
		j["shapePosOffset"]["z"] = shapeOffset.z;
	}
	END_SERIALIZE();
}

void Moon::BoxColliderComponent::Deserialize(const json& j) {
	halfExtents = glm::vec3(j["halfExtents"]["x"], j["halfExtents"]["y"], j["halfExtents"]["z"]);
	shapeOffset = glm::vec3(j["shapePosOffset"]["x"], j["shapePosOffset"]["y"], j["shapePosOffset"]["z"]);
	GetPhysicsActorAs<Physics::PhysicsBox>().Reshape(halfExtents, shapeOffset);
}