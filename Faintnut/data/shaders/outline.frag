#version 440 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform int EntityID;
uniform vec4 OutlineColor;
uniform float Radius;

layout (binding = 0) uniform usampler2D u_EntityTexture;
layout (binding = 1) uniform sampler2D u_Depth;
layout (binding = 2) uniform sampler2D WorldPositionTexture;

uniform float outline_thickness = .2;
uniform vec3 outline_colour = vec3(0, 0, 1);
uniform float outline_threshold = .5;

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
		if(depthTarget != 0.0f && depthTarget < depth)
		{
			hasHit = 1.0f;
		}

		// Mix outline with background
		float alpha = smoothstep(0.5, 0.9, hasHit * 10.0f);
		fragColor = mix(fragColor, OutlineColor, alpha);
	}
	
    if(fragColor.a > 0.1)
    {
        fragColor.a = 1.0f;
    }
    
    FragColor = mix(vec4(0), fragColor, middleSample != target && hasHit > 0.0f);
	
	
	FragColor = texture(WorldPositionTexture, gl_FragCoord.xy);

    if (FragColor.a <= outline_threshold) {
        ivec2 size = textureSize(WorldPositionTexture, 0);

        float uv_x = TexCoord.x * size.x;
        float uv_y = TexCoord.y * size.y;

        float sum = 0.0;
        for (int n = 0; n < 9; ++n) {
            uv_y = (TexCoord.y * size.y) + (outline_thickness * float(n - 4.5));
            float h_sum = 0.0;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x - (4.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x - (3.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x - (2.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x - outline_thickness, uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x, uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x + outline_thickness, uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x + (2.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x + (3.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(WorldPositionTexture, ivec2(uv_x + (4.0 * outline_thickness), uv_y), 0).a;
            sum += h_sum / 9.0;
        }

        if (sum / 9.0 >= 0.0001) {
            FragColor = vec4(outline_colour, 1);
        }
    }
}
