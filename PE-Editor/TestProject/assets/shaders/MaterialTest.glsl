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

layout(location = 0) out flat int v_EntityID;
layout(location = 1) out vec2 v_TexCoords;

void main()
{
	v_EntityID = u_MeshSubmission.EntityID;
	v_TexCoords = a_TexCoords;
	gl_Position = u_CameraBuffer.ViewProjection * u_MeshSubmission.Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

layout(location = 0) in flat int v_EntityID;
layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform MaterialValues
{
	vec4 Colour;
	float Roughness;
	float Metalness;
} u_MaterialValues;

layout(binding = 0) uniform sampler2D TestTexture;

void main()
{
	colour = texture(TestTexture, v_TexCoords);
	colour *= u_MaterialValues.Colour;
	colour.xyz *= 1.0 - u_MaterialValues.Metalness + (0.2 * u_MaterialValues.Roughness);

	if (colour.a == 0.0) { discard; }
	else { entityID = v_EntityID; }
}