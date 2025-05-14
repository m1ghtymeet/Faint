#version 440 core
precision highp float;

layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gMaterial;
layout(location = 3) out int gEntityID;
layout(location = 4) out float gEmissive;
layout(location = 5) out vec4 gVelocity;
layout(location = 6) out vec4 gUV;

in vec2 TexCoord;
in flat int entityID;
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

layout(std140, binding = 32) uniform u_MaterialUniform
{
    int u_HasAlbedo;
    vec3 m_AlbedoColor;     //  16 byte
    int u_HasMetalness;     //  32 byte
    float u_MetalnessValue; //  36 byte
    int u_HasRoughness;     //  40 byte
    float u_RoughnessValue; //  44 byte
    int u_HasAO;            //  48 byte
    float u_AOValue;        //  52 byte
    int u_HasNormal;        //  56 byte
    int u_HasDisplacement;  //  60 byte
    int u_Unlit;
    float u_Emissive;
};

void main() {
	
	// Albedo
	gAlbedo.rgb = m_AlbedoColor;
	if (u_HasAlbedo == 1) {
		vec4 albedoSample = texture(m_Albedo, TexCoord);
		gAlbedo.rgb = albedoSample.rgb /** m_AlbedoColor.rgb*/;
		gAlbedo.a = albedoSample.a;
		
		if (albedoSample.a < 0.1f) {
			discard;
		}
	}
		
	mat3 tbn = mat3(Tangent, BiTangent, Normal);
		
	// Normal
	vec3 normal = vec3(0.5f, 0.5f, 1.0f);
	if (u_HasNormal == 1) {
		normal = texture(m_Normal, TexCoord).rgb;
	}
	normal = normal * 2.0f - 1.0;
	normal = tbn * normalize(normal);
	gNormal = vec4(normal / 2.0 + 0.5, 1.0f);
	
	// Material
	float finalMetalness = u_MetalnessValue;
	if (u_HasMetalness == 1)
		finalMetalness = texture(m_Metalness, TexCoord).r;
	float finalAO = u_AOValue;
	if (u_HasAO == 1)
		finalAO = texture(m_AO, TexCoord).r;
	float finalRoughness = u_RoughnessValue;
	if (u_HasRoughness == 1)
		finalRoughness = texture(m_Roughness, TexCoord).r;
	
	gMaterial = vec4(0, 0, 0, 1);
	gMaterial.r = finalMetalness;
	gMaterial.g = finalAO;
	gMaterial.b = finalRoughness;
	
	gEntityID = aEntityID;
}