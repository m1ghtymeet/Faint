#version 440
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out flat vec2 TexCoord;
out mat4 InvProjection;
out mat4 InvView;
out vec4 lightSpacePos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 modelMatrix;
uniform mat4 lightProjectionView;

void main() {

	TexCoord = aTexCoord;
	InvProjection = inverse(projection);
	InvView = inverse(view);
	lightSpacePos = lightProjectionView * vec4(aPos, 1.0);

	gl_Position = vec4(aPos, 1.0f);
}