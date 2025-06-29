#version 440 core
layout (location = 0) in vec3 aPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 shadowMatrices[6];
uniform mat4 modelMatrix;

out vec4 FragPos;

void main() {
	//gl_Position = lightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
	FragPos = modelMatrix * vec4(aPos, 1.0);
	gl_Position = shadowMatrices[gl_InstanceID] * vec4(aPos, 1.0);
}