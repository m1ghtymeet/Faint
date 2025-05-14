#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoord;
out flat int entityID;
out vec3 Normal;
out vec3 Tangent;
out vec3 BiTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform int aEntityID;

void main() {

	TexCoord = aTexCoord;
	mat4 inverseModelMatrix = inverse(model);
	mat4 normalMatrix = transpose(inverseModelMatrix);
	
	Normal = normalize(model * vec4(aNormal, 0.0f)).xyz;
	Tangent = normalize(model * vec4(aTangent, 0.0f)).xyz;
	BiTangent = normalize(cross(Normal, Tangent));
	
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}