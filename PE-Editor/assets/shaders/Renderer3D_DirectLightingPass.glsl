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
layout(location = 1) out int f_EntityID;

// TODO: Change u_SceneData to be a shader storage buffer to allow for larger array sizes
// TODO: Before that, implement shader storage buffer abstraction in the engine
const int MAX_ACTIVE_DIR_LIGHTS = 8;
const int MAX_ACTIVE_POINT_LIGHTS = 8;
const int MAX_ACTIVE_SPOT_LIGHTS = 8;

struct DirectionalLight // vec4 for padding
{
	vec4 Direction; // w = (bool)castShadows

	vec4 Ambient; // w = shadow distance
	vec4 Diffuse; // w = min bias
	vec4 Specular; // w = max bias

	mat4 LightMatrix;
};

struct PointLight // vec4 for padding
{
	vec4 Position; // w = range

	vec4 Ambient;
	vec4 Diffuse;
	vec4 Specular;

	vec4 ShadowData; // r = minBias, g = maxBias, b = farPlane, w = (bool)castShadows
};

struct SpotLight
{
	vec4 Position; // w = range
	vec4 Direction; // w = cutoff

	vec4 Ambient; // w = outer cutoff
	vec4 Diffuse;
	vec4 Specular;

	vec4 ShadowData; // r = (bool)castShadows, g = minBias, b = maxBias

	mat4 LightMatrix;
};

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(std140, binding = 2) uniform SceneData
{
	DirectionalLight DirLights[MAX_ACTIVE_DIR_LIGHTS];
	PointLight PointLights[MAX_ACTIVE_POINT_LIGHTS];
	SpotLight SpotLights[MAX_ACTIVE_SPOT_LIGHTS];
	int ActiveDirLights;
	int ActivePointLights;
	int ActiveSpotLights;
} u_SceneData;

layout(binding = 0) uniform sampler2DArray DirectionalLightShadowMapArray;
layout(binding = 1) uniform sampler2DArray SpotLightShadowMapArray;
layout(binding = 2) uniform samplerCubeArray PointLightShadowMapArray;
layout(binding = 3) uniform sampler2D Mat_gWorldPosition;
layout(binding = 4) uniform sampler2D Mat_gWorldNormal;
layout(binding = 5) uniform sampler2D Mat_gAlbedo;
layout(binding = 6) uniform sampler2D Mat_gSpecular;
layout(binding = 7) uniform sampler2D Mat_gARM;
layout(binding = 8) uniform sampler2D Mat_gEmission;
layout(binding = 9) uniform sampler2D Mat_gMetadata;

layout(binding = 13) uniform sampler2D Mat_SSAOMap;

vec3 ViewDir;

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

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
	);
float GetShadowFactor(samplerCubeArray cubeShadowMapArray, int shadowMapLayer, vec3 lightPos, float minBias, float maxBias, vec3 Normal, float farPlane, vec3 WorldFragPos)
{
	vec3 fragToLight = WorldFragPos - lightPos;
	float currentDepth = length(fragToLight);

	float shadow = 0.0;
	float bias = max(maxBias * (1.0 - dot(Normal, fragToLight)), minBias);
	int samples = 20;

	float viewDistance = length(u_CameraBuffer.ViewPos - WorldFragPos);
	float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;

	for (int i = 0; i < samples; i++)
	{
		float closestDepth = texture(cubeShadowMapArray, vec4(fragToLight + gridSamplingDisk[i] * diskRadius, shadowMapLayer)).r;
		closestDepth *= farPlane;

		if (currentDepth - bias > closestDepth)
		{
			shadow += 1.0;
		}
	}

	shadow /= float(samples);

	return shadow;
}

float GetShadowFactor(sampler2DArray shadowMapArray, int shadowMapLayer, vec4 LightSpaceFragPos, vec3 lightPos, float minBias, float maxBias, vec3 Normal, vec3 WorldFragPos)
{
	// perspective divide
	vec3 projCoords = LightSpaceFragPos.xyz / LightSpaceFragPos.w;

	// transform to [0, 1]
	projCoords.xyz = projCoords.xyz * 0.5 + 0.5;

	if (projCoords.z > 1.0) { return 0.0; }

	// closest depth value from lights perspective
	float closestDepth = texture(shadowMapArray, vec3(projCoords.xy, shadowMapLayer)).r;

	// get fragment depth
	float currentDepth = projCoords.z;

	// check against shadow map sample
	vec3 lightDir = normalize(lightPos - WorldFragPos);
	float bias = max(maxBias * (1.0 - dot(Normal, lightDir)), minBias);

	// pcf soft shadowing
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMapArray, 0).xy;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(shadowMapArray, vec3(projCoords.xy + vec2(x, y) * texelSize, shadowMapLayer)).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	return shadow;
}

float GetShadowFactor(sampler2D shadowMap, vec4 LightSpaceFragPos, vec3 lightPos, float minBias, float maxBias, vec3 Normal, vec3 WorldFragPos)
{
	// perspective divide
	vec3 projCoords = LightSpaceFragPos.xyz / LightSpaceFragPos.w;

	// transform to [0, 1]
	projCoords.xyz = projCoords.xyz * 0.5 + 0.5;

	if (projCoords.z > 1.0) { return 0.0; }

	// closest depth value from lights perspective
	float closestDepth = texture(shadowMap, projCoords.xy).r;

	// get fragment depth
	float currentDepth = projCoords.z;

	// check against shadow map sample
	vec3 lightDir = normalize(lightPos - WorldFragPos);
	float bias = max(maxBias * (1.0 - dot(Normal, lightDir)), minBias);

	// pcf soft shadowing
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	return shadow;
}

vec3 DirectionalLightContribution(int lightIndex, vec3 MaterialAlbedo, vec3 MaterialSpecular, float MaterialShininess, vec3 Normal, vec3 WorldFragPos, float AmbientOcclusion)
{
	vec3 ambient = u_SceneData.DirLights[lightIndex].Ambient.rgb * MaterialAlbedo * AmbientOcclusion;

	// diffuse
	vec3 lightDir = normalize(-u_SceneData.DirLights[lightIndex].Direction.xyz);
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = u_SceneData.DirLights[lightIndex].Diffuse.rgb * diff * MaterialAlbedo;

	// specular
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), MaterialShininess);
	vec3 specular = (u_SceneData.DirLights[lightIndex].Specular.rgb * spec * MaterialSpecular) * diff;

	// shadow contribution
	float shadowDistance = u_SceneData.DirLights[lightIndex].Ambient.w;
	float minBias = u_SceneData.DirLights[lightIndex].Diffuse.w;
	float maxBias = u_SceneData.DirLights[lightIndex].Specular.w;

	float shadow = 0.0;
	if (bool(u_SceneData.DirLights[lightIndex].Direction.w))
	{
		shadow = GetShadowFactor(DirectionalLightShadowMapArray, lightIndex, u_SceneData.DirLights[lightIndex].LightMatrix * vec4(WorldFragPos, 1.0), -u_SceneData.DirLights[lightIndex].Direction.xyz * shadowDistance, minBias, maxBias, Normal, WorldFragPos);
	}
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 PointLightContribution(int lightIndex, vec3 MaterialAlbedo, vec3 MaterialSpecular, float MaterialShininess, vec3 Normal, vec3 WorldFragPos, float AmbientOcclusion)
{
	// attenuation
	vec4 lightPos = u_SceneData.PointLights[lightIndex].Position;
	float range = lightPos.w;
	float constant = 1.0;

	// derive unfriendly linear and quadratic values from range value
	float linear = 4.5 / range;
	float quadratic = 75.0 / (range * range);

	float dist = length(lightPos.xyz - WorldFragPos);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

	vec3 ambient = u_SceneData.PointLights[lightIndex].Ambient.rgb * MaterialAlbedo * attenuation * AmbientOcclusion;
	vec3 lightDir = normalize(lightPos.xyz - WorldFragPos);

	// diffuse
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = u_SceneData.PointLights[lightIndex].Diffuse.rgb * diff * MaterialAlbedo * attenuation;

	// specular
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), MaterialShininess);
	vec3 specular = spec * u_SceneData.PointLights[lightIndex].Specular.rgb * MaterialSpecular * diff * attenuation;

	// shadow contribution
	float minBias = u_SceneData.PointLights[lightIndex].ShadowData.r;
	float maxBias = u_SceneData.PointLights[lightIndex].ShadowData.g;
	float farPlane = u_SceneData.PointLights[lightIndex].ShadowData.b;

	float shadow = 0.0;
	if (bool(u_SceneData.PointLights[lightIndex].ShadowData.w))
	{
		shadow = GetShadowFactor(PointLightShadowMapArray, lightIndex, lightPos.xyz, minBias, maxBias, Normal, farPlane, WorldFragPos);
	}
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 SpotLightContribution(int lightIndex, vec3 MaterialAlbedo, vec3 MaterialSpecular, float MaterialShininess, vec3 Normal, vec3 WorldFragPos, float AmbientOcclusion)
{
	vec4 lightPos = u_SceneData.SpotLights[lightIndex].Position;
	vec3 lightDir = normalize(lightPos.xyz - WorldFragPos);

	// spotlight
	vec3 direction = u_SceneData.SpotLights[lightIndex].Direction.xyz;
	float cutoff = u_SceneData.SpotLights[lightIndex].Direction.w;
	float outerCutoff = u_SceneData.SpotLights[lightIndex].Ambient.w;

	float theta = dot(-lightDir, normalize(direction));
	float epsilon = cutoff - outerCutoff;
	float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);

	// attenuation
	float range = lightPos.w;
	float constant = 1.0;

	// derive unfriendly linear and quadratic values from range value
	float linear = 4.5 / range;
	float quadratic = 75.0 / (range * range);

	float dist = length(lightPos.xyz - WorldFragPos);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

	vec3 ambient = u_SceneData.SpotLights[lightIndex].Ambient.rgb * MaterialAlbedo * attenuation * AmbientOcclusion;

	// diffuse
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = u_SceneData.SpotLights[lightIndex].Diffuse.rgb * diff * MaterialAlbedo * attenuation * intensity;

	// specular
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), MaterialShininess);
	vec3 specular = spec * u_SceneData.SpotLights[lightIndex].Specular.rgb * MaterialSpecular * diff * attenuation * intensity;

	// shadow contribution
	float minBias = u_SceneData.SpotLights[lightIndex].ShadowData.g;
	float maxBias = u_SceneData.SpotLights[lightIndex].ShadowData.b;

	float shadow = 0.0;
	if (bool(u_SceneData.SpotLights[lightIndex].ShadowData.r))
	{
		shadow = GetShadowFactor(SpotLightShadowMapArray, lightIndex, u_SceneData.SpotLights[lightIndex].LightMatrix * vec4(WorldFragPos, 1.0), lightPos.xyz, minBias, maxBias, Normal, WorldFragPos);
	}
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

// Reflectance Functions
// ---------------------
vec3 PBR_DirectionalLightReflectance(int lightIndex, vec3 MaterialAlbedo, float MaterialMetallness, float MaterialRoughness, float MaterialAO, vec3 N, vec3 V, vec3 R, vec3 F0, vec3 WorldFragPos, float AmbientOcclusion)
{
	// Radiance
	vec3 L = normalize(-u_SceneData.DirLights[lightIndex].Direction.xyz);
	vec3 H = normalize(V + L);

	vec3 radiance = u_SceneData.DirLights[lightIndex].Diffuse.rgb;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, MaterialRoughness);
	float G = GeometrySmith(N, V, L, MaterialRoughness);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0, MaterialRoughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // prevent divide by zero
	vec3 specular = numerator / denominator;

	vec3 kS = F;

	// For energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light)
	// To preserve this relationship the diffuse component (kD) should equal 1.0 - kS
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - MaterialMetallness;

	float NdotL = max(dot(N, L), 0.0);

	// shadow contribution
	float shadowDistance = u_SceneData.DirLights[lightIndex].Ambient.w;
	float minBias = u_SceneData.DirLights[lightIndex].Diffuse.w;
	float maxBias = u_SceneData.DirLights[lightIndex].Specular.w;
	float shadow = 0.0;
	if (bool(u_SceneData.DirLights[lightIndex].Direction.w))
	{
		shadow = GetShadowFactor(DirectionalLightShadowMapArray, lightIndex, u_SceneData.DirLights[lightIndex].LightMatrix * vec4(WorldFragPos, 1.0), -u_SceneData.DirLights[lightIndex].Direction.xyz * shadowDistance, minBias, maxBias, N, WorldFragPos);
	}

	vec3 ambient = u_SceneData.DirLights[lightIndex].Ambient.rgb * MaterialAlbedo * MaterialAO * AmbientOcclusion;
	vec3 Lo = (kD * MaterialAlbedo / PI + specular) * (radiance * (1.0 - shadow)) * NdotL;
	return Lo + ambient;
}

vec3 PBR_PointLightReflectance(int lightIndex, vec3 MaterialAlbedo, float MaterialMetallness, float MaterialRoughness, float MaterialAO, vec3 N, vec3 V, vec3 R, vec3 F0, vec3 WorldFragPos, float AmbientOcclusion)
{
	vec4 lightPos = u_SceneData.PointLights[lightIndex].Position;

	// Radiance
	vec3 L = normalize(lightPos.xyz - WorldFragPos);
	vec3 H = normalize(V + L);

	// attenuation
	float range = lightPos.w;
	float constant = 1.0;

	// derive unfriendly linear and quadratic values from range value
	float linear = 4.5 / range;
	float quadratic = 75.0 / (range * range);

	float dist = length(lightPos.xyz - WorldFragPos);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));
	//float attenuation = 1.0 / (dist * dist);

	vec3 radiance = u_SceneData.PointLights[lightIndex].Diffuse.rgb * attenuation;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, MaterialRoughness);
	float G = GeometrySmith(N, V, L, MaterialRoughness);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0, MaterialRoughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // prevent divide by zero
	vec3 specular = numerator / denominator;

	vec3 kS = F;

	// For energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light)
	// To preserve this relationship the diffuse component (kD) should equal 1.0 - kS
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - MaterialMetallness;

	float NdotL = max(dot(N, L), 0.0);

	// shadow contribution
	float minBias = u_SceneData.PointLights[lightIndex].ShadowData.r;
	float maxBias = u_SceneData.PointLights[lightIndex].ShadowData.g;
	float farPlane = u_SceneData.PointLights[lightIndex].ShadowData.b;
	float shadow = 0.0;
	if (bool(u_SceneData.PointLights[lightIndex].ShadowData.w))
	{
		shadow = GetShadowFactor(PointLightShadowMapArray, lightIndex, lightPos.xyz, minBias, maxBias, N, farPlane, WorldFragPos);
	}

	vec3 ambient = u_SceneData.PointLights[lightIndex].Ambient.rgb * MaterialAlbedo * attenuation * MaterialAO * AmbientOcclusion;
	vec3 Lo = (kD * MaterialAlbedo / PI + specular) * (radiance * (1.0 - shadow)) * NdotL;

	return Lo + ambient;
}

vec3 PBR_SpotLightReflectance(int lightIndex, vec3 MaterialAlbedo, float MaterialMetallness, float MaterialRoughness, float MaterialAO, vec3 N, vec3 V, vec3 R, vec3 F0, vec3 WorldFragPos, float AmbientOcclusion)
{
	vec4 lightPos = u_SceneData.SpotLights[lightIndex].Position;

	// Radiance
	vec3 L = normalize(lightPos.xyz - WorldFragPos);
	vec3 H = normalize(V + L);

	// attenuation
	float range = lightPos.w;
	float constant = 1.0;

	// derive unfriendly linear and quadratic values from range value
	float linear = 4.5 / range;
	float quadratic = 75.0 / (range * range);

	float dist = length(L);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

	vec3 radiance = u_SceneData.SpotLights[lightIndex].Diffuse.rgb * attenuation;

	// spot light
	vec3 direction = u_SceneData.SpotLights[lightIndex].Direction.xyz;
	float cutoff = u_SceneData.SpotLights[lightIndex].Direction.w;
	float outerCutoff = u_SceneData.SpotLights[lightIndex].Ambient.w;

	float theta = dot(L, normalize(-direction));
	float epsilon = cutoff - outerCutoff;
	float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, MaterialRoughness);
	float G = GeometrySmith(N, V, L, MaterialRoughness);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0, MaterialRoughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // prevent divide by zero
	vec3 specular = numerator / denominator;

	vec3 kS = F;

	// For energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light)
	// To preserve this relationship the diffuse component (kD) should equal 1.0 - kS
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - MaterialMetallness;

	float NdotL = max(dot(N, L), 0.0);

	kD *= intensity;
	specular *= intensity;
	radiance *= intensity;

	// shadow contribution
	float minBias = u_SceneData.SpotLights[lightIndex].ShadowData.g;
	float maxBias = u_SceneData.SpotLights[lightIndex].ShadowData.b;
	float shadow = 0.0;
	if (bool(u_SceneData.SpotLights[lightIndex].ShadowData.r))
	{
		shadow = GetShadowFactor(SpotLightShadowMapArray, lightIndex, u_SceneData.SpotLights[lightIndex].LightMatrix * vec4(WorldFragPos, 1.0), lightPos.xyz, minBias, maxBias, N, WorldFragPos);
	}

	vec3 ambient = u_SceneData.SpotLights[lightIndex].Ambient.rgb * MaterialAlbedo * attenuation * MaterialAO * AmbientOcclusion;
	vec3 Lo = (kD * MaterialAlbedo / PI + specular) * (radiance * (1.0 - shadow)) * NdotL;
	return Lo + ambient;
}

// Lighting models
// ---------------
vec3 BlinnPhongLighting(vec3 WorldFragPos, vec3 WorldNormal, vec3 MaterialAlbedo, vec3 MaterialSpecular, float MaterialShininess, vec3 MaterialEmission, float AmbientOcclusion)
{
	vec3 result = vec3(0.0);

	// Directional lights
	for (int i = 0; i < u_SceneData.ActiveDirLights && i < MAX_ACTIVE_DIR_LIGHTS; i++)
	{
		result += DirectionalLightContribution(i, MaterialAlbedo, MaterialSpecular, MaterialShininess, WorldNormal, WorldFragPos, AmbientOcclusion);
	}

	// Point lights
	for (int i = 0; i < u_SceneData.ActivePointLights && i < MAX_ACTIVE_POINT_LIGHTS; i++)
	{
		result += PointLightContribution(i, MaterialAlbedo, MaterialSpecular, MaterialShininess, WorldNormal, WorldFragPos, AmbientOcclusion);
	}

	// Spot lights
	for (int i = 0; i < u_SceneData.ActiveSpotLights; i++)
	{
		result += SpotLightContribution(i, MaterialAlbedo, MaterialSpecular, MaterialShininess, WorldNormal, WorldFragPos, AmbientOcclusion);
	}

	result += MaterialEmission;

	return result;
}

vec3 PBRLighting(vec3 WorldFragPos, vec3 WorldNormal, vec3 MaterialAlbedo, float MaterialAO, float MaterialRoughness, float MaterialMetalness, vec3 MaterialEmission, float AmbientOcclusion)
{
	vec3 N = WorldNormal;
	vec3 V = normalize(u_CameraBuffer.ViewPos - WorldFragPos);
	vec3 R = reflect(-V, N);

	// calculate reflectance at normal incidence
	// if dialectric, use F0 of 0.04
	// if metal, use albedo colour as F0
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, MaterialAlbedo.rgb, MaterialMetalness);

	// per-light reflectance
	vec3 Lo = vec3(0.0);

	// Directional lights
	for (int i = 0; i < u_SceneData.ActiveDirLights && i < MAX_ACTIVE_DIR_LIGHTS; i++)
	{
		Lo += PBR_DirectionalLightReflectance(i, MaterialAlbedo, MaterialMetalness, MaterialRoughness, MaterialAO, N, V, R, F0, WorldFragPos, AmbientOcclusion);
	}

	// Point lights
	for (int i = 0; i < u_SceneData.ActivePointLights && i < MAX_ACTIVE_POINT_LIGHTS; i++)
	{
		Lo += PBR_PointLightReflectance(i, MaterialAlbedo, MaterialMetalness, MaterialRoughness, MaterialAO, N, V, R, F0, WorldFragPos, AmbientOcclusion);
	}

	// Spot lights
	for (int i = 0; i < u_SceneData.ActiveSpotLights; i++)
	{
		Lo += PBR_SpotLightReflectance(i, MaterialAlbedo, MaterialMetalness, MaterialRoughness, MaterialAO, N, V, R, F0, WorldFragPos, AmbientOcclusion);
	}

	vec3 result = Lo + MaterialEmission;

	return result;
}

void main()
{
	// Sample gBuffer
	vec3 WorldFragPos = texture(Mat_gWorldPosition, v_TexCoords).rgb;
	vec3 WorldNormal = texture(Mat_gWorldNormal, v_TexCoords).rgb;
	vec3 Albedo = texture(Mat_gAlbedo, v_TexCoords).rgb;

	vec4 specSample = texture(Mat_gSpecular, v_TexCoords).rgba;
	vec3 SpecularColour = specSample.rgb;
	float SpecularExponent = specSample.a;

	vec3 armSample = texture(Mat_gARM, v_TexCoords).rgb;
	float AO = armSample.r;
	float Roughness = armSample.g;
	float Metalness = armSample.b;

	vec3 Emission = texture(Mat_gEmission, v_TexCoords).rgb;
	
	vec2 metadataSample = texture(Mat_gMetadata, v_TexCoords).rg;
	int EntityID = int(metadataSample.r);
	int LightingModelIndex = int(metadataSample.g); // 0 = blinn-phong, 1 = pbr

	float AmbientOcclusion = texture(Mat_SSAOMap, v_TexCoords).r;

	f_EntityID = EntityID;
	if (LightingModelIndex == 0)
	{
		f_Colour = vec4(BlinnPhongLighting(WorldFragPos, WorldNormal, Albedo, SpecularColour, SpecularExponent, Emission, AmbientOcclusion), 1.0);
	}
	else if (LightingModelIndex == 1)
	{
		f_Colour = vec4(PBRLighting(WorldFragPos, WorldNormal, Albedo, AO, Roughness, Metalness, Emission, AmbientOcclusion), 1.0);
	}
	else
	{
		f_Colour = vec4(0.0, 0.0, 0.0, 1.0);
	}
}