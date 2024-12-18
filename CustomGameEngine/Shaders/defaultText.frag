#version 330 core
out vec4 FragColour;
in vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColour;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
	FragColour = vec4(textColour, 1.0) * sampled;
}