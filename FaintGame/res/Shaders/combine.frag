#version 440
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

layout (binding = 0) uniform sampler2D _source;
layout (binding = 1) uniform sampler2D _source2;

void main() {

	vec4 a = texture(_source, TexCoord);
	vec4 b = texture(_source2, TexCoord);

	if(b.a < 0.001f)
    {
        FragColor = a;
    }
    else
    {
        FragColor = vec4(vec3(a.rgb + (b.rgb) / 2.0), a.a);
    }
}