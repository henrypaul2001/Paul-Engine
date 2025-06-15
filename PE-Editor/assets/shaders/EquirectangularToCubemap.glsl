#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 1) uniform MeshSubmission
{
	mat4 Transform;
	int EntityID;
} u_MeshSubmission;

void main()
{
	gl_Position = u_MeshSubmission.Transform * vec4(a_Position, 1.0);
}

#type geometry
#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140, binding = 3) uniform Mat_CubeData
{
	mat4 ViewProjections[6];
	int CubemapIndex;
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
#version 450 core
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

layout(location = 0) in vec3 FragPos;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

layout(binding = 0) uniform sampler2D EquirectangularMap;

void main()
{
	vec2 uv = SampleSphericalMap(normalize(FragPos));
	vec3 colour = texture(EquirectangularMap, uv).rgb;
	FragColour = vec4(colour, 1.0);
}