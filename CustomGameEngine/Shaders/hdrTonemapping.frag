#version 330 core
layout (location = 0) out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float exposure;
uniform float gamma;

void main() {
	vec3 hdrColour = texture(screenTexture, TexCoords).rgb;

	// Reinhard tone mapping
	//vec3 mapped = hdrColour / (hdrColour + vec3(1.0));

	vec3 mapped = vec3(1.0) - exp(-hdrColour * exposure);

	// Gamma correction
	mapped = pow(mapped, vec3(1.0 / gamma));

	FragColour = vec4(mapped, 1.0);
}