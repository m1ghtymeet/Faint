#version 460 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vColor;
layout (location = 2) in ivec2 vPixelOffset;

uniform mat4 u_projectionView;
out vec3 Color;

void main() {
	Color = vColor;
	gl_Position = u_projectionView * vec4(vPosition, 1.0);
}