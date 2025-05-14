#version 450
layout (location = 0) out vec4 BaseColorOut;
layout (location = 1) out int Color2;

in vec2 TexCoord;
in flat int entityID;

uniform sampler2D image;

void main() {
	BaseColorOut = texture(image, TexCoord);
	
	Color2 = entityID;
}