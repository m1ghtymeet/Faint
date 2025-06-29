#version 460
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 WorldPositionOut;

layout (binding = 0) uniform samplerCube cubeMap;

in vec3 TexCoord;
in vec4 WorldPos;

void main() {
	vec3 skyColor = texture(cubeMap, TexCoord).rgb;
	vec3 skyLinear = pow(skyColor, vec3(2.2));
	
	FragColor = vec4(skyColor, 1);
	WorldPositionOut = vec4(WorldPos.rgb * vec3(10, 1, 10), 1.0);
}
