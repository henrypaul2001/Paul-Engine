#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Colour;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
} u_CameraBuffer;

struct VertexOutput {
	vec3 LocalPosition;
	vec4 Colour;
	float Thickness;
	float Fade;
};

layout(location = 0) out VertexOutput Output;
layout(location = 4) out flat int v_EntityID;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Colour = a_Colour;
	Output.Thickness = a_Thickness;
	Output.Fade = a_Fade;

	v_EntityID = a_EntityID;
	gl_Position = u_CameraBuffer.ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

struct VertexOutput {
	vec3 LocalPosition;
	vec4 Colour;
	float Thickness;
	float Fade;
};

layout(location = 0) in VertexOutput Input;
layout(location = 4) in flat int v_EntityID;

void main()
{
	// Calculate distance
	float distance = 1.0 - length(Input.LocalPosition.xy);
	float circle_alpha = smoothstep(0.0, Input.Fade, distance);
	circle_alpha *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);

	colour = vec4(Input.Colour.rgb, Input.Colour.a * circle_alpha);
	entityID = v_EntityID;
}