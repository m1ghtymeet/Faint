#version 440
layout (location = 0) in vec3 aPos;

out vec4 LineColor;

uniform mat4 projection;
uniform mat4 view;

void main() {
	LineColor = vec4(1, 0.5f, 0.5f, 1);
	gl_Position = projection * view * vec4(aPos, 1.0f);
}