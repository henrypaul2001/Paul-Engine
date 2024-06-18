#version 330 core
layout (location = 0) out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;
uniform sampler2D dirtMask;

uniform float bloomStrength;
uniform float dirtMaskStrength;
uniform bool useDirtMask;

void main() {
	vec3 hdr = texture(screenTexture, TexCoords).rgb;
	vec3 bloom = texture(bloomTexture, TexCoords).rgb;
	vec3 dirt = vec3(0.0);

	if (useDirtMask) {
		dirt = texture(dirtMask, TexCoords).rgb * dirtMaskStrength;
	}

	FragColour = vec4(mix(hdr, bloom + (bloom * dirt), vec3(bloomStrength)), 1.0);
}