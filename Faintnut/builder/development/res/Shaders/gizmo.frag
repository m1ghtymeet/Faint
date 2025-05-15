#version 440 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out int gEntityID;

in vec2 TexCoord;

layout (binding = 0) sampler2D depthTexture;
layout (binding = 1) sampler2D gizmoTexture;
uniform float u_Opacity;
uniform int aEntityID;

void main() {
	
	FragColor = vec4(1);
}