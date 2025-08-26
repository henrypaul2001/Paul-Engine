#context forward
#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

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
	mat4 Transform = MeshSubmissions[gl_DrawID].Transform;
	gl_Position = Transform * vec4(a_Position, 1.0);
}

#type geometry
#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140, binding = 3) uniform Mat_CubeData
{
	mat4 ViewProjections[6];
	int CubemapIndex;
	float FarPlane;
} u_CubeData;

layout(location = 0) out vec3 FragPos;

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = u_CubeData.CubemapIndex * 6 + face;
		for (int i = 0; i < 3; i++)
		{
			FragPos = gl_in[i].gl_Position.xyz;
			gl_Position = u_CubeData.ViewProjections[face] * vec4(FragPos, 1.0);
			EmitVertex();
		}
		EndPrimitive();
	}
}

#type fragment
#version 460 core

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(std140, binding = 3) uniform Mat_CubeData
{
	mat4 ViewProjections[6];
	int CubemapIndex;
	float FarPlane;
} u_CubeData;

layout(location = 0) in vec3 FragPos;

void main()
{
	float lightDistance = length(FragPos - u_CameraBuffer.ViewPos);
	lightDistance = lightDistance / u_CubeData.FarPlane;
	gl_FragDepth = lightDistance;
}