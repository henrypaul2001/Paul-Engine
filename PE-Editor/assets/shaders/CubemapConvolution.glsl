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
	int MaterialIndex;
	int padding1;
	int padding2;
};
layout(binding = 1, std430) readonly buffer MeshSubmissionSSBO {
	MeshSubmission MeshSubmissions[];
};

void main()
{
	const uint submissionIndex = gl_BaseInstance + gl_InstanceID;
	gl_Position = MeshSubmissions[submissionIndex].Transform * vec4(a_Position, 1.0);
}

#type geometry
#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140, binding = 3) uniform Mat_CubeData
{
	mat4 ViewProjections[6];
	int CubemapIndex;
} u_CubeData;

layout(location = 0) out vec3 g_FragPos;

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = u_CubeData.CubemapIndex * 6 + face;
		for (int i = 0; i < 3; i++)
		{
			g_FragPos = gl_in[i].gl_Position.xyz;
			gl_Position = u_CubeData.ViewProjections[face] * vec4(g_FragPos, 1.0);
			EmitVertex();
		}
		EndPrimitive();
	}
}

#type fragment
#version 460 core
layout(location = 0) out vec4 FragColour;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(std140, binding = 3) uniform Mat_CubeData
{
	mat4 ViewProjections[6];
	int CubemapIndex;
} u_CubeData;

layout(location = 0) in vec3 g_FragPos;
layout(binding = 0) uniform samplerCube InputCubemap;

const float PI = 3.14159265359;

void main()
{
	vec3 N = normalize(g_FragPos);
	vec3 irradiance = vec3(0.0);

	// tangent space calculation from origin point
	vec3 up		= vec3(0.0, 1.0, 0.0);
	vec3 right	= normalize(cross(up, N));
	up			= normalize(cross(N, right));

	float sampleDelta = 0.005;
	float nrSamples = 0.0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			// to world space
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(InputCubemap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / nrSamples);
	FragColour = vec4(irradiance, 1.0);
}