#version 460
out vec4 FragColor;

in vec2 TexCoord;
in mat4 InvProjection;
in mat4 InvView;
in vec4 lightSpacePos;

layout (binding = 0) uniform sampler2D m_Depth;
layout (binding = 1) uniform sampler2D m_Albedo;
layout (binding = 2) uniform sampler2D m_Normal;
layout (binding = 3) uniform sampler2D m_Material;
layout (binding = 4) uniform sampler2D WorldPositionTexture;
layout (binding = 5) uniform sampler2D ShadowMapTexture;

uniform vec3 CamPosition;
uniform float time;
uniform int lightCount;
uniform mat4 view;

struct Light {
	float posX;
	float posY;
	float posZ;
	float colorR;
	float colorG;
	float colorB;
	float radius;
	float strength;
	int type;
};

readonly restrict layout(std430, binding = 1) buffer lightsBuffer {
	Light lights[];
};

const float PI = 3.14159265359;

float map(float value, float min1, float max1, float min2, float max2) {
	float perc = (value - min1) / (max1 - min1);
	return perc * (max2 - min2) + min2;
}
vec3 Tonemap_ACES(const vec3 x) { // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness*roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 microfacetBRDF(in vec3 L, in vec3 V, vec3 N, in vec3 baseColor, in float metallic, in float fresnelReflect, in float roughness) {
    vec3 H = normalize(V + L);
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);
    vec3 F0 = vec3(0.04 * fresnelReflect); 
    F0 = mix(F0, baseColor, metallic);
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    float NdotL = max(dot(N, L), 0.0);        
    return (kD * baseColor / PI + specular);
}
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // اگر خارج از shadow map باشه، سایه نداره
    if(projCoords.z > 1.0)
        return 0.0;
        
    // Shadow bias بر اساس زاویه نور
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // PCF بهبود یافته
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMapTexture, 0);
    int pcfRange = 2; // 5x5 PCF
    
    for(int x = -pcfRange; x <= pcfRange; ++x) {
        for(int y = -pcfRange; y <= pcfRange; ++y) {
            float pcfDepth = texture(ShadowMapTexture, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
        }    
    }
    
    int totalSamples = (pcfRange * 2 + 1) * (pcfRange * 2 + 1);
    shadow /= float(totalSamples);
    
    return shadow;
}

vec3 GetDirectLighting(vec3 lightPos, vec3 lightColor, float radius, float strength, vec3 Normal, vec3 WorldPos, vec3 baseColor, float roughness, float metallic, vec3 viewPos) {       
    float fresnelReflect = 1.0;
	vec3 viewDir = normalize(viewPos - WorldPos);
	float lightRadiance = strength;
	vec3 lightDir = normalize(lightPos - WorldPos);   
	float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));
	float irradiance = max(dot(lightDir, Normal), 0.0) ;
	irradiance *= lightAttenuation * lightRadiance;
	vec3 brdf = microfacetBRDF(lightDir, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness);
    return brdf * irradiance * clamp(lightColor, 0, 1);
}

vec3 GetSpotlightLighting2(vec3 lightPos, vec3 lightDir, vec3 lightColor, float radius, float strength, float innerAngle, float outerAngle, vec3 Normal, vec3 WorldPos, vec3 baseColor, float roughness, float metallic, vec3 viewPos) {       
    float fresnelReflect = 1.0;
	vec3 viewDir = normalize(viewPos - WorldPos);
	float lightRadiance = strength;
	vec3 toLight = normalize(lightPos - WorldPos);   
	float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));

	// Spotlight effect
	float spotIntensity = dot(toLight, normalize(-lightDir));
	float spotFactor = smoothstep(outerAngle, innerAngle, spotIntensity);
	
	float irradiance = max(dot(toLight, Normal), 0.0);
	irradiance *= lightAttenuation * lightRadiance * spotFactor;
	
	vec3 brdf = microfacetBRDF(toLight, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness);
    return brdf * irradiance * clamp(lightColor, 0, 1);
}
vec3 GetSpotlightLighting(
    vec3 lightPos, vec3 lightDir, vec3 lightColor, 
    float radius, float strength, 
    float innerAngle, float outerAngle, 
    vec3 Normal, vec3 WorldPos, 
    vec3 baseColor, float roughness, float metallic, 
    vec3 viewPos, mat4 LightViewProj
) {       
    float fresnelReflect = 1.0;
    vec3 viewDir = normalize(viewPos - WorldPos);
    float lightRadiance = strength;
    vec3 toLight = normalize(lightPos - WorldPos);
    
    // **Compute standard distance attenuation**
    float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));

    // **Spotlight intensity based on angle**
    float spotIntensity = dot(toLight, normalize(-lightDir));
    float spotFactor = smoothstep(outerAngle, innerAngle, spotIntensity);

    // **Project world position into light space (same as cookie projection)**
    vec4 lightSpacePos = LightViewProj * vec4(WorldPos, 1.0);
    vec2 projectedUV = lightSpacePos.xy / lightSpacePos.w;
    projectedUV = projectedUV * 0.5 + 0.5;

    // **Depth factor to match cookie projection**
    float depthFactor = 1.0 / max(lightSpacePos.w, 0.001);
    projectedUV *= depthFactor; // Perspective warp

    // **Weaken light over distance (same as cookie)**
    float distanceFactor = clamp(1.0 - (length(lightPos - WorldPos) / radius), 0.0, 1.0);
    distanceFactor *= distanceFactor; // Quadratic falloff for smooth fade

    // **Blend spotFactor with perspective-based fade**
    spotFactor *= distanceFactor;

    // **Compute final light intensity**
    float irradiance = max(dot(toLight, Normal), 0.0);
    irradiance *= lightAttenuation * lightRadiance * spotFactor;

    vec3 brdf = microfacetBRDF(toLight, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness);
    return brdf * irradiance * clamp(lightColor, 0, 1);
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}
vec3 filmPixel(vec2 uv) {
    mat2x3 uvs = mat2x3(uv.xxx, uv.yyy) + mat2x3(vec3(0, 0.1, 0.2), vec3(0, 0.3, 0.4));
    return fract(sin(uvs * vec2(12.9898, 78.233) * time) * 43758.5453);
}

void main() {

	vec3 viewPos = InvView[3].xyz;
	float clipSpaceXMin = 0.0f;
	float clipSpaceXMax = 1.0f;
	float clipSpaceYMin = 0.0f;
	float clipSpaceYMax = 1.0f;
	
	vec4 baseColor = texture(m_Albedo, TexCoord);
	baseColor.rgb = pow(baseColor.rgb, vec3(2.2f));
	vec3 normal = texture(m_Normal, TexCoord).rgb * 2.0 - 1.0;
	vec4 materialSample = texture(m_Material, TexCoord);
	float metallic = materialSample.r;
    float ao = materialSample.g;
	float roughness = materialSample.b;
	float unlit = materialSample.a;
	
	vec3 WorldPos = texture(WorldPositionTexture, TexCoord).rgb;
	
	//FragColor.rgb = vec3(1.0 - ShadowCalculation(lightSpacePos * vec4(WorldPos, 1), normal, normalize(vec3(10.0f, 15.0f, 10.0f) - WorldPos)));
	//FragColor.rgb = WorldPos;
	//
	//return;
	
    vec3 directLighting = vec3(0);
	for (int i = 0; i < lightCount; i++) {
		Light light = lights[i];
		vec3 lightPos = vec3(light.posX, light.posY, light.posZ);
		vec3 lightColor = vec3(light.colorR, light.colorG, light.colorB);
		float radius = light.radius;
		float strength = light.strength;
		
		float shadow = ShadowCalculation(lightSpacePos * vec4(WorldPos, 1), normal, normalize(lightPos - WorldPos));
		
		if (light.type == 0) { // Point
			directLighting += GetDirectLighting(lightPos, lightColor, radius, strength, normal, WorldPos, baseColor.rgb, roughness, metallic, viewPos) /* (1.0 - shadow)*/;
		}
		if (light.type == 1) { // Directional
			
		}
		else if (light.type == 2) { // Spot
		//	vec3 forward = -normalize(vec3(InvView[2].xyz));
		//	vec3 spotLightDir = normalize(lightPos - (viewPos - forward));
		//	vec3 spotLightingFactor = GetSpotlightLighting(lightPos, vec3(0, 1, 0), lightColor, radius, strength, light.InnerAngle, light.OuterAngle, normal, WorldPos, albedo, roughness, metallic, viewPos, mat4(1));
		//	
		//	directLighting += spotLightingFactor;
		}
	}
    FragColor.rgb = directLighting;
	
	// Tone mapping
    FragColor.rgb = mix(FragColor.rgb, Tonemap_ACES(FragColor.rgb), 1.0);   
	
	// Gamma correct
	FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
	FragColor.rgb = mix(FragColor.rgb, Tonemap_ACES(FragColor.rgb), 0.35); 
	
	// Vigentte
    vec2 uv = TexCoord;
    uv *= 1.0 - uv.yx;
    float vig = uv.x * uv.y * 62.0;
    vig = pow(vig, 0.05);
    FragColor.rgb *= vec3(vig);
	
	// Noise
	vec2 viewportSize = textureSize(m_Albedo, 0);
	vec2 filmRes = viewportSize;
	vec2 coord = gl_FragCoord.xy;
	vec2 rest = modf(uv * filmRes, coord);
	vec3 noise00 = filmPixel(coord / filmRes);
	vec3 noise01 = filmPixel((coord + vec2(0, 1)) / filmRes);
	vec3 noise10 = filmPixel((coord + vec2(1, 0)) / filmRes);
	vec3 noise11 = filmPixel((coord + vec2(1, 1)) / filmRes);
	vec3 noise = mix(mix(noise00, noise01, rest.y), mix(noise10, noise11, rest.y), rest.x) * vec3(0.7, 0.6, 0.8);
	float noiseSpeed = 15.0;
	float x = rand(uv + rand(vec2(int(time * noiseSpeed), int(-time * noiseSpeed))));
	float noiseFactor = 0.035;
	
	// Some more YOLO tone mapping
	//FragColor.rgb = mix(FragColor.rgb, Tonemap_ACES(FragColor.rgb), 0.75);
	
	// Adding Noise
	FragColor.rgb = FragColor.rgb + (x * -noiseFactor) + (noiseFactor / 2);
	
	// Constast
    float constrast = 1.15;
    FragColor.rgb = FragColor.rgb * constrast;
	
	// Brightness
    FragColor.rgb -= vec3(0.020);
	
	vec3 lightVec = vec3(1, 1, 1) - WorldPos;
	
	float dist = length(lightVec);
	float a = 3.0;
	float b = 0.7;
	float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);
	
	float ambient = 0.20f;
	
	vec3 lightDir = normalize(vec3(10.0f, 15.0f, 10.0f));
	float diffuse = max(dot(normal, lightDir), 0.0f);
	
	float specular = 0.0f;
	if (diffuse != 0.0f) {
		float specularLight = 0.50f;
		vec3 viewDir = normalize(CamPosition - WorldPos);
		vec3 halfwayVec = normalize(viewDir + lightDir);
		float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
		specular = specAmount * specularLight;
	}
	
	float shadow = 0.0f;
	vec4 fragPosLight = lightSpacePos * vec4(WorldPos, 1.0f);
	vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
	if (lightCoords.z <= 1.0f) {
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;
		float bias = max(0.025f * (1.0f - dot(normal, lightDir)), 0.0005f);
	
		int sampleRadius = 2;
		vec2 pixelSize = 1.0 / textureSize(ShadowMapTexture, 0);
		for (int y = -sampleRadius; y <= sampleRadius; y++) {
			for (int x = -sampleRadius; x <= sampleRadius; x++) {
				float closestDepth = texture(ShadowMapTexture, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;
			}
		}
		shadow /= pow((sampleRadius * 2 + 1), 2);
	}
	
	vec4 result = (baseColor * (diffuse * (1.0f - shadow) + ambient) + roughness * specular * (1.0f - shadow)) * vec4(1, 1, 1, 1);
	//FragColor = result;
	//FragColor = texture(ShadowMapTexture, TexCoord);
	//FragColor = fragPosLight;
}