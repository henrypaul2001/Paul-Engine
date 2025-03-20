#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Colour;
layout (location = 2) in vec2 a_TexCoords;
layout (location = 3) in int a_EntityID;

layout (std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
} u_CameraBuffer;

layout (location = 0) out vec4 v_Colour;
layout (location = 1) out vec2 v_TexCoords;
layout (location = 2) out flat int v_EntityID;

void main()
{
	v_Colour = a_Colour;
	v_TexCoords = a_TexCoords;
	v_EntityID = a_EntityID;

	gl_Position = u_CameraBuffer.ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 colour;
layout (location = 1) out int entityID;

layout(location = 0) in vec4 v_Colour;
layout(location = 1) in vec2 v_TexCoords;
layout(location = 2) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_FontAtlas;

float screenPxRange() {
	const float pxRange = 2.0;
	vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlas, 0));
	vec2 screenTexSize = vec2(1.0) / fwidth(v_TexCoords);
	return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
} 

void main() {
	vec4 texColour = v_Colour * texture(u_FontAtlas, v_TexCoords);

	vec3 msd = texture(u_FontAtlas, v_TexCoords).rgb;
	float sd = median(msd.r, msd.g, msd.b);
	float screenPxDistance = screenPxRange() * (sd - 0.5);
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (opacity == 0.0) { discard; }

	vec4 bgColour = vec4(0.0);
	colour = mix(bgColour, v_Colour, opacity);
	if (colour.a == 0.0) { discard; }
	entityID = v_EntityID;
}