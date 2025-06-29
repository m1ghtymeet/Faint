#version 460 core
layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gMaterial;
layout(location = 3) out vec4 WorldPositionOut;
layout(location = 4) out int gEntityID;

in vec2 TexCoord;
in flat int entityID;
in vec4 WorldPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 BiTangent;

// Material
layout (binding = 0) uniform sampler2D m_Albedo;
layout (binding = 1) uniform sampler2D m_Metalness;
layout (binding = 2) uniform sampler2D m_Roughness;
layout (binding = 3) uniform sampler2D m_AO;
layout (binding = 4) uniform sampler2D m_Normal;
layout (binding = 5) uniform sampler2D m_Displacement;
uniform int aEntityID;
uniform int materialCount;

//struct MaterialData {
//	int hasAlbedo;
//	int hasNormal;
//	int hasMetalness;
//	int hasRoughness;
//};
//readonly restrict layout(std430, binding = 2) buffer materialsBuffer {
//	MaterialData materials[];
//};

void main() {
	
	//MaterialData material = materials[0];
	// Albedo
	//gAlbedo.rgb ;
	//if (material.hasAlbedo == 1) {
		vec4 albedoSample = texture(m_Albedo, TexCoord);
		gAlbedo.rgb = albedoSample.rgb;
		gAlbedo.a = albedoSample.a;
		
		if (albedoSample.a < 0.1f) {
			discard;
		}
	//}
		
	mat3 tbn = mat3(Tangent, BiTangent, Normal);
		
	// Normal
	vec3 normal = vec3(0.5f, 0.5f, 1.0f);
	//if (u_HasNormal == 1) {
		normal = texture(m_Normal, TexCoord).rgb;
	//}
	normal = normalize(normal * 2.0f - 1.0);
	normal = normalize(tbn * normal);
	gNormal = vec4(normal * 0.5 + 0.5, 1.0f);
	
	// Material
	//float finalMetalness = u_MetalnessValue;
	//if (u_HasMetalness == 1)
		float finalMetalness = texture(m_Metalness, TexCoord).r;
	//float finalAO = u_AOValue;
	//if (u_HasAO == 1)
		float finalAO = texture(m_AO, TexCoord).r;
	//float finalRoughness = u_RoughnessValue;
	//if (u_HasRoughness == 1)
		float finalRoughness = texture(m_Roughness, TexCoord).r;
	
	gMaterial = vec4(0, 0, 0, 1);
	gMaterial.r = finalMetalness;
	gMaterial.g = finalAO;
	gMaterial.b = finalRoughness;
	
	WorldPositionOut = WorldPos;
	
	gEntityID = aEntityID;
}