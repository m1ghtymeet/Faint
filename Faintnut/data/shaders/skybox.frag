#version 460 core

layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform samplerCube cubeMap;

in vec3 TexCoord;

void contrastAdjust( inout vec4 color, in float c) {
    float t = 0.5 - c * 0.5;
    color.rgb = color.rgb * c + t;
}

void main() {

	FragColor.rgb = texture(cubeMap, TexCoord).rgb;
	//FragColor.rgb *= vec3(1.0);
	
	//vec3 desaturdatedColor = vec3(dot(vec3(0.200, 0.500, 0.100), FragColor.rgb));
	//FragColor.rgb = mix(desaturdatedColor, FragColor.rgb, 0.25);

	//contrastAdjust(FragColor, 1.125);
	//FragColor.rgb *= vec3(0.5);
	//FragColor.a = 1.0f;
}
