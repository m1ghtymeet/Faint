#version 440
out vec4 FragColor;

in vec2 TexCoord;

layout (binding = 0) uniform sampler2D _source;
layout (binding = 1) uniform sampler2D _source2;

void main() {

	vec4 a = texture(_source, TexCoord);
	vec4 b = texture(_source2, TexCoord);

	float luminance = max(b.r, max(b.g, b.b));
    FragColor = vec4(mix(a.rgb, b.rgb, luminance), a.a);
}