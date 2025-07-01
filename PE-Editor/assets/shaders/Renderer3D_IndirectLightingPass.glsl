#context deferred
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

layout(location = 0) out vec4 f_Colour;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(binding = 3) uniform sampler2D Mat_gWorldPosition;
layout(binding = 4) uniform sampler2D Mat_gWorldNormal;
layout(binding = 5) uniform sampler2D Mat_gAlbedo;
layout(binding = 6) uniform sampler2D Mat_gSpecular;
layout(binding = 7) uniform sampler2D Mat_gARM;
layout(binding = 8) uniform sampler2D Mat_gMetadata;

// Global IBL
layout(binding = 10) uniform samplerCube IrradianceMap;
layout(binding = 11) uniform samplerCube PrefilterMap;
layout(binding = 12) uniform sampler2D BRDFLut;

layout(binding = 13) uniform sampler2D Mat_SSAOMap;
layout(binding = 14) uniform sampler2D Mat_DirectLightingPass;
layout(binding = 15) uniform sampler2D Mat_SSRUVMap;

// PBR Utility Functions
// ---------------------
const float PI = 3.14159265359;
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

float GeometrySchlickGGX(float NdotV, float Roughness)
{
	float r = (Roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float Roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, Roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, Roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0, float Roughness)
{
	return F0 + (max(vec3(1.0 - Roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// IBL Functions
// -------------
vec3 IBLDiffuseContribution(samplerCube irradianceMap, vec3 MaterialAlbedo, vec3 N)
{
	vec3 irradiance = texture(irradianceMap, N).rgb;
	return irradiance * MaterialAlbedo;
}
vec3 IBLSpecularContribution(samplerCube prefilterMap, vec3 MaterialSpecular, float MaterialShininess, vec3 R)
{
	const float MAX_REFLECTION_LOD = 6.0; // maxMipLevels = 7 in EnvironmentMap::PrefilterEnvironmentMap();

	// Shininess to mip level
	float gloss = MaterialShininess / 256.0;
	float mipLevel = (1.0 - gloss) * MAX_REFLECTION_LOD;

	vec3 prefilteredColour = textureLod(prefilterMap, R, mipLevel).rgb;
	return prefilteredColour * MaterialSpecular;
}

vec3 PBR_IBLDiffuseContribution(samplerCube irradianceMap, vec3 MaterialAlbedo, vec3 fresnelFactor, float MaterialMetalness, vec3 N)
{
	vec3 kD = 1.0 - fresnelFactor;
	kD *= 1.0 - MaterialMetalness;

	vec3 irradiance = texture(irradianceMap, N).rgb;
	return kD * (irradiance * MaterialAlbedo);
}
vec3 PBR_IBLSpecularContribution(samplerCube prefilterMap, float MaterialRoughness, vec3 fresnelFactor, vec2 brdf, vec3 R)
{
	const float MAX_REFLECTION_LOD = 6.0; // maxMipLevels = 7 in EnvironmentMap::PrefilterEnvironmentMap();
	vec3 prefilteredColour = textureLod(prefilterMap, R, MaterialRoughness * MAX_REFLECTION_LOD).rgb;
	return prefilteredColour * (fresnelFactor * brdf.x + brdf.y);
}

// SSR Functions
// -------------
// TODO: figure out a way to incorporate a simplified roughness blur without brdf in blinn-phong. Or, maybe just use brdf in both lighting models
vec3 SSRSpecularContribution(vec2 sampleUV)
{
	return texture(Mat_DirectLightingPass, sampleUV).rgb;
}
vec3 PBR_SSRSpecularContribution(vec2 sampleUV, vec2 brdf, vec3 fresnelFactor)
{
	vec3 reflectedColour = texture(Mat_DirectLightingPass, sampleUV).rgb;
	return reflectedColour * (fresnelFactor * brdf.x + brdf.y);
}

// Lighting models
// ---------------
const float minIBLContribution = 0.0;
vec3 IndirectLighting(vec3 Albedo, vec3 Specular, float SpecularExponent, vec2 ssrUVCoords, float ssrContribution, float MaterialAO, float AmbientOcclusion, vec3 N, vec3 R)
{
	// Get indirect diffuse contribution
	vec3 iblDiffuse = IBLDiffuseContribution(IrradianceMap, Albedo, N);

	// Get SSR specular contribution
	vec3 ssrSpecular = SSRSpecularContribution(ssrUVCoords);

	// Get IBL Specular contribution
	vec3 iblSpecular = IBLSpecularContribution(PrefilterMap, Specular, SpecularExponent, R);

	// Get final specular blend
	vec3 finalSpecularContribution = mix(ssrSpecular, iblSpecular, max(minIBLContribution, 1.0 - ssrContribution));

	return (iblDiffuse + finalSpecularContribution) * MaterialAO * AmbientOcclusion;
}
vec3 PBR_IndirectLighting(vec3 Albedo, float Roughness, float Metalness, float NdotV, vec2 ssrUVCoords, float ssrContribution, float MaterialAO, float AmbientOcclusion, vec3 N, vec3 R)
{
	// Get indirect diffuse contribution
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, Albedo, Metalness);
	const vec3 F = FresnelSchlick(NdotV, F0, Roughness);
	const vec2 brdf = texture(BRDFLut, vec2(NdotV, Roughness)).rg;

	vec3 iblDiffuse = PBR_IBLDiffuseContribution(IrradianceMap, Albedo, F, Metalness, N);

	// Get SSR specular contribution
	vec3 ssrSpecular = PBR_SSRSpecularContribution(ssrUVCoords, brdf, F);

	// Get IBL specular contribution
	vec3 iblSpecular = PBR_IBLSpecularContribution(PrefilterMap, Roughness, F, brdf, R);

	// Get final specular blend
	vec3 finalSpecularContribution = mix(ssrSpecular, iblSpecular, max(minIBLContribution, 1.0 - ssrContribution));

	return (iblDiffuse + finalSpecularContribution) * MaterialAO * AmbientOcclusion;
}

void main()
{
	// Sample gBuffer
	const vec3 WorldFragPos = texture(Mat_gWorldPosition, v_TexCoords).rgb;
	const vec3 WorldNormal = texture(Mat_gWorldNormal, v_TexCoords).rgb;
	const vec3 Albedo = texture(Mat_gAlbedo, v_TexCoords).rgb;

	const vec3 armSample = texture(Mat_gARM, v_TexCoords).rgb;
	const float MaterialAO = armSample.r;
	const float Roughness = armSample.g;
	const float Metalness = armSample.b;
	
	const vec2 metadataSample = texture(Mat_gMetadata, v_TexCoords).rg;
	const int EntityID = int(metadataSample.r);
	const int LightingModelIndex = int(metadataSample.g); // 0 = blinn-phong, 1 = pbr

	const float AmbientOcclusion = texture(Mat_SSAOMap, v_TexCoords).r;

	const vec3 N = WorldNormal;
	const vec3 V = normalize(u_CameraBuffer.ViewPos - WorldFragPos);
	const vec3 R = reflect(-V, N);

	const float NdotV = max(dot(N, V), 0.0);

	const vec4 ssrUVSample = texture(Mat_SSRUVMap, v_TexCoords);
	const vec2 ssrUVCoords = ssrUVSample.rg;
	const float ssrContribution = ssrUVSample.b;

	// Order of importance for indirect lighting contribution
	// 0. IBL Diffuse contribution
	// 1. Screen space reflections for specular factor
	// 2. Fallback to IBL specular contribution
	
	vec3 indirectLighting = vec3(0.0);
	if (LightingModelIndex == 0) // Blinn-Phong
	{
		const vec4 specSample = texture(Mat_gSpecular, v_TexCoords).rgba;
		const vec3 SpecularColour = specSample.rgb;
		const float SpecularExponent = specSample.a;

		indirectLighting = IndirectLighting(Albedo, SpecularColour, SpecularExponent, ssrUVCoords, ssrContribution, MaterialAO, AmbientOcclusion, N, R);
	}
	else if (LightingModelIndex == 1) // PBR
	{
		indirectLighting = PBR_IndirectLighting(Albedo, Roughness, Metalness, NdotV, ssrUVCoords, ssrContribution, MaterialAO, AmbientOcclusion, N, R);
	}

	vec3 currentColour = texture(Mat_DirectLightingPass, v_TexCoords).rgb;
	f_Colour = vec4(currentColour + indirectLighting, 1.0);
}