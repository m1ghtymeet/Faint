#version 440 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

layout (binding = 0) uniform sampler2D _source;

void main()
{
    vec3 color = texture(_source, TexCoord).rgb;
	
	FragColor = texture(_source, TexCoord);
}