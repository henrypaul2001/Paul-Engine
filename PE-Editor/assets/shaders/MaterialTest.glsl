#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in vec2 a_TexScale;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
} u_CameraBuffer;

struct VertexOutput {
	vec4 Colour;
};

layout(location = 0) out VertexOutput Output;
layout(location = 4) out flat int v_EntityID;

void main()
{
	Output.Colour = a_Colour;
	v_EntityID = a_EntityID;

	gl_Position = u_CameraBuffer.ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

struct VertexOutput {
	vec4 Colour;
};

layout(location = 0) in VertexOutput Input;
layout(location = 4) in flat int v_EntityID;

layout(std140, binding = 1) uniform MaterialValues
{
	vec4 Colour;
	float Roughness;
	float Metalness;
} u_MaterialValues;

void main()
{
	colour = u_MaterialValues.Colour;
	colour.xyz *= 1.0 - u_MaterialValues.Metalness + (0.2 * u_MaterialValues.Roughness);

	if (colour.a == 0.0) { discard; }
	else { entityID = v_EntityID; }
}