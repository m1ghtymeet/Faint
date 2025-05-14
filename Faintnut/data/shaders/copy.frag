#version 440
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

layout (binding = 0) uniform sampler2D _source;

void main() {

    FragColor = texture(_source, TexCoord);
}