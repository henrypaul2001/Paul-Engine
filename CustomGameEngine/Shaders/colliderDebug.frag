#version 330 core
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

uniform vec3 Colour;

void main() {
	FragColour = vec4(Colour, 1.0);
}