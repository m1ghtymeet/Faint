#version 440 core

layout (location = 0) out vec4 FragColor;

in flat vec2 TexCoord;

uniform int EntityID;
uniform vec4 OutlineColor;
uniform float Radius;

layout (binding = 0) uniform usampler2D u_EntityTexture;
layout (binding = 1) uniform sampler2D u_Depth;

void main()
{
    int target = EntityID;
    const float TAU = 6.28318530;
	const float steps = 32.0;
    
	float radius = Radius;
	vec2 uv = TexCoord;
    
	// sample middle
	uint middleSample = texture(u_EntityTexture, uv).r;
	float depth = texture(u_Depth, uv).r;
	
    // Correct aspect ratio
    vec2 aspect = 1.0 / vec2(textureSize(u_EntityTexture, 0));
    
	float hasHit = 0.0f;
	vec4 fragColor = vec4(0.0, 0.0, 0.0, 0.0f);
	for (float i = 0.0; i < TAU; i += TAU / steps) 
    {
		// Sample image in a circular pattern
        vec2 offset = vec2(sin(i), cos(i)) * aspect * radius;
		
		// We dont want to sample outside the viewport
		vec2 sampleUv = uv + offset;
		sampleUv.x = clamp(sampleUv.x, 0.0, 0.999);
		sampleUv.y = clamp(sampleUv.y, 0.0, 0.999);

		uint col = texture(u_EntityTexture, sampleUv).r;
		float depthTarget = texture(u_Depth, sampleUv).r;
		if(col == target && depthTarget != 0.0f && depthTarget < depth)
		{
			hasHit = 1.0f;
		}

		// Mix outline with background
		float alpha = smoothstep(0.5, 0.9, int(col != target) * hasHit * 10.0f);
		fragColor = mix(fragColor, OutlineColor, alpha);
	}
	
    if(fragColor.a > 0.1)
    {
        fragColor.a = 1.0f;
    }
    
    FragColor = mix(vec4(0), fragColor, middleSample != target && hasHit > 0.0f);
}
