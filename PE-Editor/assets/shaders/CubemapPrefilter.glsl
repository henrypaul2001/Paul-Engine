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
	gl_Position = MeshSubmissions[gl_DrawID].Transform * vec4(a_Position, 1.0);
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
} u_CubeData;

layout(std140, binding = 4) uniform Mat_PrefilterParams
{
	float Roughness;
	int FaceWidth;
	int FaceHeight;
} u_Params;

layout(location = 0) in vec3 g_FragPos;

layout(binding = 0) uniform samplerCube InputCubemap;

const float PI = 3.14159265359;
const int SAMPLE_COUNT = 4096 * 4;

float DistributionGGX(vec3 N, vec3 H, float Roughness)
{
	float a = Roughness * Roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N) {
	return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float Roughness)
{
	float a = Roughness * Roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// spherical coordinates to cartesian
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// tangent to world space
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = normalize(cross(N, tangent));

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

void main()
{
	vec3 N = normalize(g_FragPos);

	// make the simplifying assumption that V equals R equals the normal 
	vec3 R = N;
	vec3 V = R;

	vec3 prefilteredColour = vec3(0.0);
	float totalWeight = 0.0;

	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		// generate sample vector biased towards preferred alignment direction
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, u_Params.Roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0)
		{
			// sample from mip level based on roughness
			float D = DistributionGGX(N, H, u_Params.Roughness);
			float NdotH = max(dot(N, H), 0.0);
			float HdotV = max(dot(H, V), 0.0);
			float pdf = D * NdotH / max((4.0 * HdotV) + 0.0001, 0.0001);

			float saTexel = 4.0 * PI / (6.0 * u_Params.FaceWidth * u_Params.FaceHeight);
			float saSample = max(1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001), 0.0001);

			float mipLevel = u_Params.Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
			mipLevel = clamp(mipLevel, 0.0, log2(float(max(u_Params.FaceWidth, u_Params.FaceHeight))));

			prefilteredColour += textureLod(InputCubemap, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	prefilteredColour = prefilteredColour / totalWeight;
	FragColour = vec4(prefilteredColour, 1.0);
}