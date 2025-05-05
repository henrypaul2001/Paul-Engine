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
} u_CameraBuffer;

layout(std140, binding = 1) uniform MeshSubmission
{
	mat4 Transform;
	int EntityID;
} u_MeshSubmission;

layout(location = 0) out vec3 FragPos;

void main()
{
	vec4 worldPos = u_MeshSubmission.Transform * vec4(a_Position, 1.0);
	FragPos = worldPos.xyz;
	gl_Position = u_CameraBuffer.ViewProjection * worldPos;
}

#type fragment
#version 450 core

//#pragma glslang depthReplacing
//layout(depth_any) out float gl_FragDepth;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
} u_CameraBuffer;

layout(std140, binding = 2) uniform Data
{
	float FarPlane;
} u_Data;

layout(location = 0) in vec3 FragPos;

void main()
{
	float lightDistance = length(FragPos - u_CameraBuffer.ViewPos);
	lightDistance = lightDistance / u_Data.FarPlane;
	gl_FragDepth = lightDistance;
}