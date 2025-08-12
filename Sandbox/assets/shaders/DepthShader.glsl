#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
} u_CameraBuffer;

layout(std140, binding = 1) uniform MeshSubmission
{
	mat4 Transform;
	int EntityID;
} u_MeshSubmission;

void main()
{
	gl_Position = u_CameraBuffer.ViewProjection * u_MeshSubmission.Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main()
{
	// do nothing (write to depth buffer)
}