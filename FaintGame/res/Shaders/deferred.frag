#version 440 core

out vec4 FragColor;

in vec2 TexCoord;
in mat4 InvProjection;
in mat4 InvView;

layout (binding = 0) uniform sampler2D m_Depth;
layout (binding = 1) uniform sampler2D m_Albedo;
layout (binding = 2) uniform sampler2D m_Normal;
layout (binding = 3) uniform sampler2D m_Material;

const int MaxLight = 42;
uniform int LightCount = 0;

struct Light {
    vec3 Color;
    vec3 Position;
    int Type; // 0 = Point, 1 = Directional, 2 = Spot
    vec3 Direction;
    float Strength;
	float Radius;
	float OuterAngle;
    float InnerAngle;
    int CastShadow; // For SpotLight for now
    int ShadowMapID;
    mat4 Transform;
};

struct DirectionalLight
{
    vec3 Direction;
    vec3 Color;
    float CascadeDepth[4];
    mat4 LightTransforms[4];
    int Shadow;
};

uniform vec3 CamPosition;

uniform sampler2D ShadowMaps[4];

uniform Light Lights[MaxLight];
uniform DirectionalLight a_DirectionalLight;
uniform float gamma;
uniform float time;

// Converts depth to World space coords.
vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = InvProjection * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = InvView * viewSpacePosition;

    return worldSpacePosition.xyz;
}

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

float D_GGX(float NoH, float roughness) {
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float NoH2 = NoH * NoH;
  float b = (NoH2 * (alpha2 - 1.0) + 1.0);
  return alpha2 / (PI * b * b);
}

float G1_GGX_Schlick(float NdotV, float roughness) {
  //float r = roughness; // original
  float r = 0.5 + 0.5 * roughness; // Disney remapping
  float k = (r * r) / 2.0;
  float denom = NdotV * (1.0 - k) + k;
  return NdotV / denom;
}

float G_Smith(float NoV, float NoL, float roughness) {
  float g1_l = G1_GGX_Schlick(NoL, roughness);
  float g1_v = G1_GGX_Schlick(NoV, roughness);
  return g1_l * g1_v;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 microfacetBRDF(in vec3 L, in vec3 V, in vec3 N, in vec3 baseColor, in float metallicness, in float fresnelReflect, in float roughness, in vec3 WorldPos) {
  vec3 H = normalize(V + L); // half vector
  // all required dot products
  float NoV = clamp(dot(N, V), 0.0, 1.0);
  float NoL = clamp(dot(N, L), 0.0, 1.0);
  float NoH = clamp(dot(N, H), 0.0, 1.0);
  float VoH = clamp(dot(V, H), 0.0, 1.0);
  // F0 for dielectics in range [0.0, 0.16]
  // default FO is (0.16 * 0.5^2) = 0.04
  vec3 f0 = vec3(0.16 * (fresnelReflect * fresnelReflect));
  // f0 = vec3(0.125);
  // in case of metals, baseColor contains F0
  f0 = mix(f0, baseColor, metallicness);
  // specular microfacet (cook-torrance) BRDF
  vec3 F = fresnelSchlick(VoH, f0);
  float D = D_GGX(NoH, roughness);
  float G = G_Smith(NoV, NoL, roughness);
  vec3 spec = (D * G * F) / max(4.0 * NoV * NoL, 0.001);

  // diffuse
  vec3 notSpec = vec3(1.0) - F; // if not specular, use as diffuse
  notSpec *= 1.0 - metallicness; // no diffuse for metals
  vec3 diff = notSpec * baseColor / PI;
  spec *= 1.05;
  vec3 result = diff + spec;

  return result;
}
vec3 GetDirectLighting(vec3 lightPos, vec3 lightColor, float radius, float strength, vec3 Normal, vec3 WorldPos, vec3 baseColor, float roughness, float metallic, vec3 viewPos) {
	float fresnelReflect = 1.0; // 0.5 is what they used for box, 1.0 for demon
	vec3 viewDir = normalize(viewPos - WorldPos);
	float lightRadiance = strength * 1;// * 1.25;
    vec3 lightDir = normalize(lightPos - WorldPos);
	//float lightAttenuation = 1.0f;
	float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));
	// lightAttenuation = clamp(lightAttenuation, 0.0, 0.9); // THIS IS WRONG, but does stop super bright region around light source and doesn't seem to affect anything else...
	float irradiance = max(dot(lightDir, Normal), 0.0) ;
	irradiance *= lightAttenuation * lightRadiance;
	vec3 brdf = microfacetBRDF(lightDir, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness, WorldPos);
	return brdf * irradiance * clamp(lightColor, 0, 1);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}


float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

int GetCSMDepth(float depth)
{
    int shadowmap = -1;
    // Get CSM depth
    for (int i = 0; i < 4; i++)
    {
        float CSMDepth = a_DirectionalLight.CascadeDepth[i];

        if (depth < CSMDepth + 0.0001f)
        {
            shadowmap = i;
            break;
        }
    }

    return shadowmap;
}
float SampleShadowMap(sampler2D shadowMap, vec2 coords, float compare)
{
    return compare < texture(shadowMap, coords.xy).r ? 1.0f : 0.0f;
}

float SampleShadowMapLinear(sampler2D shadowMap, vec2 coords, float compare, vec2 texelSize)
{
    //return SampleShadowMap(shadowMap, coords.xy, compare);
    vec2 pixelPos = coords / texelSize + vec2(0.5);
    vec2 fracPart = fract(pixelPos);
    vec2 startTexel = (pixelPos - fracPart) * texelSize;

    float blTexel = SampleShadowMap(shadowMap, startTexel, compare);
    float brTexel = SampleShadowMap(shadowMap, startTexel + vec2(texelSize.x, 0.0), compare);
    float tlTexel = SampleShadowMap(shadowMap, startTexel + vec2(0.0, texelSize.y), compare);
    float trTexel = SampleShadowMap(shadowMap, startTexel + texelSize, compare);

    float mixA = mix(blTexel, tlTexel, fracPart.y);
    float mixB = mix(brTexel, trTexel, fracPart.y);

    return mix(mixA, mixB, fracPart.x);
}

float ShadowCalculation(vec3 FragPos, vec3 normal)
{
    // Get Depth
    float depth = length(FragPos - CamPosition);
    int shadowmap = GetCSMDepth(depth);
    if (shadowmap == -1)
        return 1.0;

    vec4 fragPosLightSpace = a_DirectionalLight.LightTransforms[shadowmap] * vec4(FragPos, 1.0f);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = 0.0;
    float bias = max(0.005 * (1.0 - dot(normal, a_DirectionalLight.Direction)), 0.0005);
    //float pcfDepth = texture(ShadowMaps[shadowmap], vec3(projCoords.xy, currentDepth), bias);

    //return SampleShadowMap(ShadowMaps[shadowmap], projCoords.xy, currentDepth - bias);
    
    if (shadowmap <= 4)
    {
        const float NUM_SAMPLES = 4.f;
        const float SAMPLES_START = (NUM_SAMPLES - 1.0f) / 2.0f;
        const float NUM_SAMPLES_SQUARED = NUM_SAMPLES * NUM_SAMPLES;
        vec2 texelSize = 1.0 / vec2(2048, 2048);

        float result = 0.0f;
        for (float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0f)
        {
            for (float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0f)
            {
                vec2 coordsOffset = vec2(x, y) * texelSize;
                result += SampleShadowMapLinear(ShadowMaps[shadowmap], projCoords.xy + coordsOffset, currentDepth - bias, texelSize);
            }
        }

        return result / NUM_SAMPLES_SQUARED;
    }
    else
    {
        return SampleShadowMap(ShadowMaps[shadowmap], projCoords.xy, currentDepth - bias);
    }
   
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

	vec3 worldPos = WorldPosFromDepth(texture(m_Depth, TexCoord).r);
	if (texture(m_Depth, TexCoord).r == 1) {
		FragColor = vec4(0, 0, 0, 0);
		return;
	}
	
	vec3 albedo = texture(m_Albedo, TexCoord).rgb;
	albedo.rgb = pow(albedo.rgb, vec3(2.2f)); // Gamma Added!!!!!
	vec3 normal = texture(m_Normal, TexCoord).rgb * 2.0 - 1.0;
	vec4 materialSample = texture(m_Material, TexCoord);
	float metallic = materialSample.r;
    float ao = materialSample.g;
	float roughness = materialSample.b;
	float unlit = materialSample.a;

    // Position from depth reconsturction
	float z = texture(m_Depth, TexCoord).x;
    vec2 clipSpaceTexCoord = TexCoord;
	clipSpaceTexCoord.x = map(clipSpaceTexCoord.x, clipSpaceXMin, clipSpaceXMax, 0.0, 1.0);
	clipSpaceTexCoord.y = map(clipSpaceTexCoord.y, clipSpaceYMin, clipSpaceYMax, 0.0, 1.0);
	vec4 clipSpacePosition = vec4(clipSpaceTexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = InvProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = InvView * viewSpacePosition;
    vec3 WorldPos = worldSpacePosition.xyz;
	
	/*vec3 N = normal;
    vec3 V = normalize(CamPosition	- worldPos);
    vec3 R = reflect(-V, N);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
	
	vec3 eyeDirection = normalize(CamPosition - worldPos);
	
	//vec3 V = normalize(CamPosition - worldPos);

	vec3 Lo = vec3(0.0f);
	for (int i = 0; i < LightCount; i++) {
	
		Light light = Lights[i];
		vec3 lightPos = light.Position;
		vec3 lightColor = light.Color;
		vec3 wi = normalize(lightPos - worldPos);
		//vec3 cosTheta = max(dot(N, wi), 0.0);
		
		float distance = length(lightPos - worldPos);
        float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColor * attenuation ;
		
		vec3 L = normalize(lightPos - worldPos);
		vec3 H = normalize(V + L);
		
		vec3 F0 = vec3(0.04f);
		F0 = mix(F0, albedo, metallic);
		vec3 F = fresnelSchlick(max(dot(H, V), 0), F0);
		
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0) * max(dot(N, L), 0) + 0.0001f;
		vec3 specular = numerator / denominator;
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		
		kD *= 1.0 - metallic;
		
		float NdotL = max(dot(N, L), 0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	float shadow = 1.0f;
	if (true)
    {
        vec3 L = normalize(a_DirectionalLight.Direction);

        float attenuation = 1.0f;

        L = normalize(a_DirectionalLight.Direction);

        //if(a_DirectionalLight.Shadow > 0.1f)
        //{
        //    shadow *= ShadowCalculation(worldPos, N);
        //}

        vec3 radiance = a_DirectionalLight.Color * attenuation;

        vec3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
		
		vec3 F0 = vec3(0.04f);
		F0 = mix(F0, albedo, metallic);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadow;
    }
	
	vec3 ambient = vec3(0.03) * albedo * ao;
	FragColor.rgb = ambient + Lo;*/

    vec3 directLighting = vec3(0);
	for (int i = 0; i < LightCount; i++)
	{
		Light light = Lights[i];
		vec3 lightPos = light.Position;
		vec3 lightColor = light.Color;
		float radius = light.Radius;
		float strength = light.Strength;
		directLighting += GetDirectLighting(lightPos, lightColor, radius, strength, normal, worldPos, albedo, roughness, metallic, viewPos);
	}
    FragColor.rgb = directLighting;

	// Tone mapping
	FragColor.rgb = mix(FragColor.rgb, Tonemap_ACES(FragColor.rgb), 1.0);
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / 2.2));
    FragColor.rgb = mix(FragColor.rgb, Tonemap_ACES(FragColor.rgb), 0.1235);
	
	// Vigentte
    vec2 uv = TexCoord;
    uv *= 1.0 - uv.yx;
    float vig = uv.x * uv.y * 15.0;
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
	FragColor.rgb = mix(FragColor.rgb, Tonemap_ACES(FragColor.rgb), 0.75);
	
	// Adding Noise
	FragColor.rgb = FragColor.rgb + (x * -noiseFactor) + (noiseFactor / 2);
	
	// Constast
    float constrast = 1.15;
    FragColor.rgb = FragColor.rgb * constrast;
	
	// Brightness
    FragColor.rgb -= vec3(0.020);
}