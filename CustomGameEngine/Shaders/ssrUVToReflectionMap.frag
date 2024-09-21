#version 430 core

layout (location = 0) out vec4 FragColour;

uniform sampler2D colourMap;
uniform sampler2D uvMap;

in vec2 TexCoords;

void main() {
	vec4 sampleUV = texture(uvMap, TexCoords);
	vec4 reflectedColour = texture(colourMap, sampleUV.xy);

	float alpha = clamp(sampleUV.b, 0.0, 1.0);

	FragColour = vec4(mix(vec3(0.0), reflectedColour.rgb, alpha), alpha);
}