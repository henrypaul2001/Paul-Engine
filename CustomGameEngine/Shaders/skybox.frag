#version 330 core
layout (location = 0) out vec4 FragColour;

in vec3 TexCoords;

uniform samplerCube cubemap;

void main() {
	FragColour = texture(cubemap, TexCoords);
}