#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
} u_CameraBuffer;

layout(location = 0) out vec4 v_Colour;
layout(location = 1) out flat int v_EntityID;

void main()
{
	v_Colour = a_Colour;
	v_EntityID = a_EntityID;

	gl_Position = u_CameraBuffer.ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

layout(location = 0) in vec4 v_Colour;
layout(location = 1) in flat int v_EntityID;

void main()
{
	colour = v_Colour;
	if (colour.a == 0.0) {
		discard;
	}
	else {
		entityID = v_EntityID;
	}
}