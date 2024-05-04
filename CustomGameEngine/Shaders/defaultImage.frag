#version 330 core
out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D image;

void main() {
	FragColour = texture(image, TexCoords);
}