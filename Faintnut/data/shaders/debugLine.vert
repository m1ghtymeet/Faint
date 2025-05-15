#version 440
layout (location = 0) in vec3 aPos;

uniform vec3 startPos;
uniform vec3 endPos;

uniform mat4 projection;
uniform mat4 view;

void main() {
	vec3 pos = mix(startPos, endPos, aPos.x);
	gl_Position = projection * view * vec4(pos, 1.0f);
}