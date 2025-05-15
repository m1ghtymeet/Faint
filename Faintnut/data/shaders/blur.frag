#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D u_Texture;
uniform float u_BlurSize;

void main() {
    float offset = 1.0 / u_BlurSize;
    vec3 result = vec3(0.0);
    result += texture(u_Texture, TexCoord + vec2(-4.0 * offset, 0.0)).rgb * 0.05;
    result += texture(u_Texture, TexCoord + vec2(-2.0 * offset, 0.0)).rgb * 0.25;
    result += texture(u_Texture, TexCoord).rgb * 0.4;
    result += texture(u_Texture, TexCoord + vec2(2.0 * offset, 0.0)).rgb * 0.25;
    result += texture(u_Texture, TexCoord + vec2(4.0 * offset, 0.0)).rgb * 0.05;

    FragColor = vec4(result, 1.0);
}