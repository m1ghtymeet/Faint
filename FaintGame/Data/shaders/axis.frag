#version 460
in vec3 vColor;
out vec4 FragOut;

void main() {
	FragOut = vec4(vColor, 1.0);
}