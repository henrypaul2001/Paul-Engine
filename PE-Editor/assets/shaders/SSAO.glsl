// designed to be used after a deferred geometry pass, but doesn't contribute to the gBuffer itself
#context forward
#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 1) out vec2 v_TexCoords;

void main()
{
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 450 core

layout(location = 0) out float f_SSAO;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform Mat_SSAOData
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec2 SourceResolution;
	float Radius;
	float Bias;
	int KernelSize;
} u_SSAOData;

const int MAX_SAMPLES = 64;
layout(std140, binding = 4) uniform Mat_SSAOSamples
{
	vec4 Samples[MAX_SAMPLES];	// vec4 for padding, w is unused. Could use a smaller array of vec4 where the 4th component is used as the x component of the following "vec3" 
						// but that would create a bit of a mess on the CPU side with reflection. This is fine for now
} u_SSAOSamples;

layout(binding = 0) uniform sampler2D Mat_gWorldPosition;
layout(binding = 1) uniform sampler2D Mat_gWorldNormal;
layout(binding = 2) uniform sampler2D Mat_NoiseTexture;

vec2 NoiseScale = u_SSAOData.SourceResolution / 4.0;

void main()
{
	mat4 View = u_SSAOData.ViewMatrix;
	mat4 Projection = u_SSAOData.ProjectionMatrix;

	// Read inputs
	vec3 ViewFragPos = vec3(View * vec4(texture(Mat_gWorldPosition, v_TexCoords).xyz, 1.0));
	vec3 ViewNormal = mat3(View) * texture(Mat_gWorldNormal, v_TexCoords).xyz;
	vec3 randomVec = normalize(texture(Mat_NoiseTexture, v_TexCoords * NoiseScale).xyz * 2.0 - 1.0);

	// Create TBN
	vec3 tangent = normalize(randomVec - ViewNormal * dot(randomVec, ViewNormal));
	vec3 bitangent = cross(ViewNormal, tangent);
	mat3 TBN = mat3(tangent, bitangent, ViewNormal);

	// Iterate sample kernel and calculate occlusion factor
	float occlusion = 0.0;
	for (int i = 0; i < u_SSAOData.KernelSize && i < MAX_SAMPLES; i++)
	{
		// sample position
		vec3 samplePos = TBN * u_SSAOSamples.Samples[i].xyz;
		samplePos = ViewFragPos + samplePos * u_SSAOData.Radius;

		// project sample position
		vec4 offset = vec4(samplePos, 1.0);
		offset = Projection * offset;		 // view to clip space
		offset.xyz /= offset.w;				 // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // to range 0.0, 1.0

		vec3 sampleFragPos = vec3(View * vec4(texture(Mat_gWorldPosition, offset.xy).xyz, 1.0));
		float sampleDepth = sampleFragPos.z;

		// range check and accumulate occlusion factor
		float rangeCheck = smoothstep(0.0, 1.0, u_SSAOData.Radius / abs(ViewFragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + u_SSAOData.Bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / min(u_SSAOData.KernelSize, MAX_SAMPLES));
	f_SSAO = occlusion;
}