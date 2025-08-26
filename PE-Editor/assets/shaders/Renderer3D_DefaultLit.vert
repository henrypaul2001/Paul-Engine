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
    int padding0;
    int padding1;
    int padding2;
};
layout(binding = 1, std430) readonly buffer MeshSubmissionSSBO {
	MeshSubmission MeshSubmissions[];
};

struct VertexData
{
	vec3 WorldFragPos;
	vec3 Normal;
	vec2 TexCoords;

	mat3 TBN;
};

layout(location = 0) out flat int v_EntityID;
layout(location = 1) out VertexData v_VertexData;

void main()
{
	mat4 Transform = MeshSubmissions[gl_DrawID].Transform;

	mat3 normalMatrix = mat3(transpose(inverse(Transform)));
	v_VertexData.WorldFragPos = vec3(Transform * vec4(a_Position, 1.0));
	v_VertexData.Normal = normalMatrix * a_Normal;
	v_VertexData.TexCoords = a_TexCoords;

	// Tangent space
	vec3 T = normalMatrix * a_Tangent;
	vec3 N = v_VertexData.Normal;
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	v_VertexData.TBN = mat3(T, B, N);

	v_EntityID = MeshSubmissions[gl_DrawID].EntityID;

	gl_Position = u_CameraBuffer.Projection * u_CameraBuffer.View * vec4(v_VertexData.WorldFragPos, 1.0);
}