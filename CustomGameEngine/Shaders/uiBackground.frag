#version 330 core

layout (location = 0) out vec4 FragColour;

in vec2 TexCoords;

uniform vec4 colour;

uniform vec4 borderColour;
uniform float borderThickness;
uniform bool drawBorder;

void main() {

	vec4 finalColour = colour;

	if (drawBorder) {
		if (TexCoords.x <= (borderThickness) || TexCoords.x >= (1.0 - borderThickness) || TexCoords.y <= (borderThickness) || TexCoords.y >= (1.0 - borderThickness)) {
			finalColour = borderColour;
		}
	}

	FragColour = finalColour;
}