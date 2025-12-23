#pragma once
#include <glm/glm.hpp>

class ICamera {
public:
	virtual const glm::mat4& GetViewMatrix() const = 0;
	virtual const glm::mat4& GetProjectionMatrix() const = 0;
	virtual const glm::vec3& GetPosition() const = 0;
	virtual ~ICamera() = default;
};