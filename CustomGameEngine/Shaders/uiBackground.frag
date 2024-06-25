#version 330 core

layout (location = 0) out vec4 FragColour;

in vec2 TexCoords;

uniform vec4 Colour;

void main() {
	FragColour = Colour;
}