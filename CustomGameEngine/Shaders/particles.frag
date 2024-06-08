#version 330 core
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

in vec2 TexCoords;
in vec4 ParticleColour;

//out vec4 Colour;

uniform sampler2D sprite;

void main() {
	FragColour = texture(sprite, TexCoords) * ParticleColour;
}