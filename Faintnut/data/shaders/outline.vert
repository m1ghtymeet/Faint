#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_projectionView;
uniform mat4 modelMatrix;
uniform float outlineWidth;

void main() {
	vec3 worldPos = vec3(modelMatrix * vec4(aPos, 1.0));
	vec3 worldNormal = normalize(mat3(transpose(inverse(modelMatrix))) * aNormal);
	
	vec3 outlinePos = worldPos + worldNormal * (outlineWidth - 1.0) * 0.1;
	
	gl_Position = u_projectionView * vec4(outlinePos, 1.0);
}