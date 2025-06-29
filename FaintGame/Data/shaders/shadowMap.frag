#version 440 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main() {
	float lightDistance = length(FragPos.xyz - vec3(0, 0, 0));
    lightDistance = lightDistance / 25.5f;
    gl_FragDepth = lightDistance;
}