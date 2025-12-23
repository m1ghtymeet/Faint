#include "CCamera.h"
#include <Scene/Entity.h>

Moon::CameraComponent::CameraComponent(Entity& p_owner) : AComponent(p_owner) {
    camera.SetProjectionMatrix(glm::perspective(camera.m_fieldOfView, 1280.0f / 720.0f, camera.m_nearPlane, camera.m_farPlane));
}

void Moon::CameraComponent::Update(int renderWidth, int renderHeight) {
    //if (dirty) {
        //camera.SetProjectionMatrix(glm::perspective(camera.m_fieldOfView, (float)renderWidth / (float)renderHeight, GetNear(), GetFar()));
		m_w = (float)renderWidth;
		m_h = (float)renderHeight;
       // dirty = false;
    //}
}

void Moon::CameraComponent::MarkAsDirty() {
    camera.SetProjectionMatrix(glm::perspective(camera.m_fieldOfView, m_w / m_h, GetNear(), GetFar()));
    dirty = true;
}

void Moon::CameraComponent::SetFov(float p_value) {
    camera.m_fieldOfView = p_value;
}

void Moon::CameraComponent::SetNear(float p_value) {
    camera.m_nearPlane = p_value;
}

void Moon::CameraComponent::SetFar(float p_value) {
    camera.m_farPlane = p_value;
}

float Moon::CameraComponent::GetFov() const {
    return camera.m_fieldOfView;
}

float Moon::CameraComponent::GetNear() const {
    return camera.m_nearPlane;
}

float Moon::CameraComponent::GetFar() const {
    return camera.m_farPlane;
}

json Moon::CameraComponent::Serialize() {
    BEGIN_SERIALIZE();
    j["Near"] = camera.m_nearPlane;
    j["Far"] = camera.m_farPlane;
    j["FOV"] = camera.m_fieldOfView;
    END_SERIALIZE();
}

void Moon::CameraComponent::Deserialize(const json& j) {
    if (j.contains("FOV")) camera.m_fieldOfView = j["FOV"];
    if (j.contains("Near")) camera.m_nearPlane = j["Near"];
    if (j.contains("Far")) camera.m_farPlane = j["Far"];
    camera.SetPosition(owner.transform->GetLocalPosition());
    camera.SetRotationQ(owner.transform->GetLocalRotation());
}