#version 440
layout (location = 0) out vec4 FragColor;
layout (location = 1) out int EntityID;

uniform vec4 Color;
uniform float _Opacity;
uniform int _EntityID;

in vec4 LineColor;

void main() {
	
	if (Color == vec4(0, 0, 0, 0))
		FragColor = LineColor * vec4(1, 1, 1, _Opacity);
	else
		FragColor = Color;
	
	EntityID = int(_EntityID);
}