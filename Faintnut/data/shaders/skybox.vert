#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 modelMatrix;

void main() {

	TexCoord = aPos;
	gl_Position = projection * view * modelMatrix * vec4(aPos, 1.0);
}