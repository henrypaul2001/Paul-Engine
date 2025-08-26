#context forward
#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

struct MeshSubmission
{
	mat4 Transform;
	int EntityID;
};
layout(binding = 1, std430) readonly buffer MeshSubmissionSSBO {
	MeshSubmission MeshSubmissions[];
};

void main()
{
	gl_Position = u_CameraBuffer.Projection * u_CameraBuffer.View * MeshSubmissions[gl_DrawID].Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core

void main()
{
	// do nothing (write to depth buffer)
}