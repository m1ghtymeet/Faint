#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out flat int entityID;

uniform mat4 viewProjection;
uniform mat4 model;
uniform int aEntityID;

void main() {
	TexCoord = aTexCoord;
	gl_Position = viewProjection * model * vec4(aPos, 1.0f);
	
	entityID = aEntityID;
}