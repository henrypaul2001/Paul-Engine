#context forward
#type vertex
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
	int MaterialIndex;
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
layout(location = 1) out flat uint v_MaterialIndex;
layout(location = 2) out VertexData v_VertexData;

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
	v_MaterialIndex = uint(MeshSubmissions[gl_DrawID].MaterialIndex);

	gl_Position = u_CameraBuffer.Projection * u_CameraBuffer.View * vec4(v_VertexData.WorldFragPos, 1.0);
}

#type fragment
#version 460 core
#extension GL_ARB_bindless_texture : require
layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

const int MAX_ACTIVE_DIR_LIGHTS = 8;
const int MAX_ACTIVE_POINT_LIGHTS = 8;
const int MAX_ACTIVE_SPOT_LIGHTS = 8;

struct VertexData
{
	vec3 WorldFragPos;
	vec3 Normal;
	vec2 TexCoords;

	mat3 TBN;
};

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

layout(location = 0) in flat int v_EntityID;
layout(location = 1) in flat uint v_MaterialIndex;
layout(location = 2) in VertexData v_VertexData;

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
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

struct MaterialValues
{
	vec4 Albedo;
	vec4 Specular;

	vec2 TextureScale;
	float Shininess;
	float HeightScale;

	vec3 EmissionColour;
	float EmissionStrength;

	sampler2D AlbedoMap;		// guaranteed resident
	sampler2D SpecularMap;		// guaranteed resident

	sampler2D NormalMap;		// not guaranteed resident (check int flag before sampling)
	sampler2D DisplacementMap;	// not guaranteed resident (check int flag before sampling)

	sampler2D EmissionMap;		// guaranteed resident
	int UseNormalMap;
	int UseDisplacementMap;
};
layout(binding = 3, std430) readonly buffer IMat_MaterialSSBO
{
	MaterialValues[] MaterialBuffer;
};

layout(binding = 0) uniform sampler2DArray DirectionalLightShadowMapArray;
layout(binding = 1) uniform sampler2DArray SpotLightShadowMapArray;
layout(binding = 2) uniform samplerCubeArray PointLightShadowMapArray;

// Global IBL
layout(binding = 10) uniform samplerCube IrradianceMap;
layout(binding = 11) uniform samplerCube PrefilterMap;

vec2 ScaledTexCoords;
vec3 ViewDir;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
	);
float GetShadowFactor(samplerCubeArray cubeShadowMapArray, int shadowMapLayer, vec3 lightPos, float minBias, float maxBias, vec3 Normal, float farPlane)
{
	vec3 fragToLight = v_VertexData.WorldFragPos - lightPos;
	float currentDepth = length(fragToLight);

	float shadow = 0.0;
	float bias = max(maxBias * (1.0 - dot(Normal, fragToLight)), minBias);
	int samples = 20;

	float viewDistance = length(u_CameraBuffer.ViewPos - v_VertexData.WorldFragPos);
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

float GetShadowFactor(sampler2DArray shadowMapArray, int shadowMapLayer, vec4 LightSpaceFragPos, vec3 lightPos, float minBias, float maxBias, vec3 Normal)
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
	vec3 lightDir = normalize(lightPos - v_VertexData.WorldFragPos);
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

float GetShadowFactor(sampler2D shadowMap, vec4 LightSpaceFragPos, vec3 lightPos, float minBias, float maxBias, vec3 Normal)
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
	vec3 lightDir = normalize(lightPos - v_VertexData.WorldFragPos);
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

const float minLayers = 0.0;
const float maxLayers = 32.0;
vec2 ParallaxMapping(vec2 texCoords, vec3 tangentViewDir)
{
	// calculate number of depth layers
	float numLayers = mix(maxLayers, minLayers, dot(vec3(0.0, 0.0, 1.0), tangentViewDir));
	float layerDepth = 1.0 / numLayers;

	float currentLayerDepth = 0.0f;

	// amount to shift the texture coordinates per layer
	vec2 P = tangentViewDir.xy * MaterialBuffer[v_MaterialIndex].HeightScale;
	vec2 deltaTexCoords = (P / numLayers);

	// initial sample
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(MaterialBuffer[v_MaterialIndex].DisplacementMap, currentTexCoords).r;

	float i = 0.0;
	while (currentLayerDepth < currentDepthMapValue && i < 32.0) {
		// shift coords along direction of P
		currentTexCoords -= deltaTexCoords;

		currentDepthMapValue = texture(MaterialBuffer[v_MaterialIndex].DisplacementMap, currentTexCoords).r;

		currentLayerDepth += layerDepth;
		i += 1.0;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for interpolation
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(MaterialBuffer[v_MaterialIndex].DisplacementMap, prevTexCoords).r - currentLayerDepth + layerDepth;

	// interpolate
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

vec3 DirectionalLightContribution(int lightIndex, vec3 MaterialAlbedo, vec3 MaterialSpecular, vec3 Normal)
{
	vec3 ambient = u_SceneData.DirLights[lightIndex].Ambient.rgb * MaterialAlbedo;

	// diffuse
	vec3 lightDir = normalize(-u_SceneData.DirLights[lightIndex].Direction.xyz);
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = u_SceneData.DirLights[lightIndex].Diffuse.rgb * diff * MaterialAlbedo;

	// specular
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), MaterialBuffer[v_MaterialIndex].Shininess);
	vec3 specular = (u_SceneData.DirLights[lightIndex].Specular.rgb * spec * MaterialSpecular) * diff;

	// shadow contribution
	float shadowDistance = u_SceneData.DirLights[lightIndex].Ambient.w;
	float minBias = u_SceneData.DirLights[lightIndex].Diffuse.w;
	float maxBias = u_SceneData.DirLights[lightIndex].Specular.w;

	float shadow = 0.0;
	if (bool(u_SceneData.DirLights[lightIndex].Direction.w))
	{
		shadow = GetShadowFactor(DirectionalLightShadowMapArray, lightIndex, u_SceneData.DirLights[lightIndex].LightMatrix * vec4(v_VertexData.WorldFragPos, 1.0), -u_SceneData.DirLights[lightIndex].Direction.xyz * shadowDistance, minBias, maxBias, Normal);
	}
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 PointLightContribution(int lightIndex, vec3 MaterialAlbedo, vec3 MaterialSpecular, vec3 Normal)
{
	// attenuation
	vec4 lightPos = u_SceneData.PointLights[lightIndex].Position;
	float range = lightPos.w;
	float constant = 1.0;

	// derive unfriendly linear and quadratic values from range value
	float linear = 4.5 / range;
	float quadratic = 75.0 / (range * range);

	float dist = length(lightPos.xyz - v_VertexData.WorldFragPos);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

	vec3 ambient = u_SceneData.PointLights[lightIndex].Ambient.rgb * MaterialAlbedo * attenuation;
	vec3 lightDir = normalize(lightPos.xyz - v_VertexData.WorldFragPos);

	// diffuse
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = u_SceneData.PointLights[lightIndex].Diffuse.rgb * diff * MaterialAlbedo * attenuation;

	// specular
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), MaterialBuffer[v_MaterialIndex].Shininess);
	vec3 specular = spec * u_SceneData.PointLights[lightIndex].Specular.rgb * MaterialSpecular * diff * attenuation;

	// shadow contribution
	float minBias = u_SceneData.PointLights[lightIndex].ShadowData.r;
	float maxBias = u_SceneData.PointLights[lightIndex].ShadowData.g;
	float farPlane = u_SceneData.PointLights[lightIndex].ShadowData.b;

	float shadow = 0.0;
	if (bool(u_SceneData.PointLights[lightIndex].ShadowData.w))
	{
		shadow = GetShadowFactor(PointLightShadowMapArray, lightIndex, lightPos.xyz, minBias, maxBias, Normal, farPlane);
	}
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 SpotLightContribution(int lightIndex, vec3 MaterialAlbedo, vec3 MaterialSpecular, vec3 Normal)
{
	vec4 lightPos = u_SceneData.SpotLights[lightIndex].Position;
	vec3 lightDir = normalize(lightPos.xyz - v_VertexData.WorldFragPos);

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

	float dist = length(lightPos.xyz - v_VertexData.WorldFragPos);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

	vec3 ambient = u_SceneData.SpotLights[lightIndex].Ambient.rgb * MaterialAlbedo * attenuation;

	// diffuse
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = u_SceneData.SpotLights[lightIndex].Diffuse.rgb * diff * MaterialAlbedo * attenuation * intensity;

	// specular
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), MaterialBuffer[v_MaterialIndex].Shininess);
	vec3 specular = spec * u_SceneData.SpotLights[lightIndex].Specular.rgb * MaterialSpecular * diff * attenuation * intensity;

	// shadow contribution
	float minBias = u_SceneData.SpotLights[lightIndex].ShadowData.g;
	float maxBias = u_SceneData.SpotLights[lightIndex].ShadowData.b;

	float shadow = 0.0;
	if (bool(u_SceneData.SpotLights[lightIndex].ShadowData.r))
	{
		shadow = GetShadowFactor(SpotLightShadowMapArray, lightIndex, u_SceneData.SpotLights[lightIndex].LightMatrix * vec4(v_VertexData.WorldFragPos, 1.0), lightPos.xyz, minBias, maxBias, Normal);
	}
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

// IBL Functions
// -------------
vec3 CalculateAmbienceFromIBL(samplerCube prefilterMap, samplerCube irradianceMap, vec3 MaterialAlbedo, vec3 MaterialSpecular, float MaterialShininess, vec3 N, vec3 V, vec3 R)
{
	const float MAX_REFLECTION_LOD = 6.0; // maxMipLevels = 7 in EnvironmentMap::PrefilterEnvironmentMap();

	// Shininess to mip level
	float gloss = MaterialShininess / 256.0;
	float mipLevel = (1.0 - gloss) * MAX_REFLECTION_LOD;

	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = irradiance * MaterialAlbedo;

	vec3 prefilteredColour = textureLod(prefilterMap, R, mipLevel).rgb;
	vec3 specular = prefilteredColour * MaterialSpecular;

	return diffuse + specular;
}

void main()
{
	ViewDir = normalize(u_CameraBuffer.ViewPos - v_VertexData.WorldFragPos);
	vec3 TangentViewDir = transpose(v_VertexData.TBN) * ViewDir;

	ScaledTexCoords = v_VertexData.TexCoords;
	ScaledTexCoords *= MaterialBuffer[v_MaterialIndex].TextureScale;

	// Apply parallax mapping to TexCoords
	if (MaterialBuffer[v_MaterialIndex].UseDisplacementMap != 0)
	{
		ScaledTexCoords = ParallaxMapping(ScaledTexCoords, TangentViewDir);
		//ScaledTexCoords = clamp(ScaledTexCoords, 0.0, 1.0);
	}

	// If no texture is provided, these samplers will sample a default 1x1 white texture.
	// This results in no change to the underlying material value when they are multiplied
	vec3 AlbedoSample = pow(texture(MaterialBuffer[v_MaterialIndex].AlbedoMap, ScaledTexCoords).rgb, vec3(u_CameraBuffer.Gamma));
	vec3 EmissionSample = pow(texture(MaterialBuffer[v_MaterialIndex].EmissionMap, ScaledTexCoords).rgb, vec3(u_CameraBuffer.Gamma));
	vec3 SpecularSample = vec3(texture(MaterialBuffer[v_MaterialIndex].SpecularMap, ScaledTexCoords).r);

	vec3 Normal = normalize(v_VertexData.Normal);
	if (MaterialBuffer[v_MaterialIndex].UseNormalMap != 0)
	{
		Normal = texture(MaterialBuffer[v_MaterialIndex].NormalMap, ScaledTexCoords).rgb;
		Normal = normalize(Normal * 2.0 - 1.0);
		Normal = normalize(v_VertexData.TBN * Normal);
	}

	vec3 MaterialAlbedo = AlbedoSample * MaterialBuffer[v_MaterialIndex].Albedo.rgb;
	vec3 MaterialEmission = EmissionSample * (MaterialBuffer[v_MaterialIndex].EmissionColour * MaterialBuffer[v_MaterialIndex].EmissionStrength);
	vec3 MaterialSpecular = SpecularSample * MaterialBuffer[v_MaterialIndex].Specular.rgb;

	vec3 Result = vec3(0.0);

	// Directional lights
	for (int i = 0; i < u_SceneData.ActiveDirLights && i < MAX_ACTIVE_DIR_LIGHTS; i++)
	{
		Result += DirectionalLightContribution(i, MaterialAlbedo, MaterialSpecular, Normal);
	}

	// Point lights
	for (int i = 0; i < u_SceneData.ActivePointLights && i < MAX_ACTIVE_POINT_LIGHTS; i++)
	{
		Result += PointLightContribution(i, MaterialAlbedo, MaterialSpecular, Normal);
	}

	// Spot lights
	for (int i = 0; i < u_SceneData.ActiveSpotLights; i++)
	{
		Result += SpotLightContribution(i, MaterialAlbedo, MaterialSpecular, Normal);
	}

	colour = vec4(Result, MaterialBuffer[v_MaterialIndex].Albedo.a);

	// Emission
	colour.rgb += MaterialEmission;

	// Global IBL
	colour.rgb += CalculateAmbienceFromIBL(PrefilterMap, IrradianceMap, MaterialAlbedo, MaterialSpecular, MaterialBuffer[v_MaterialIndex].Shininess, Normal, ViewDir, reflect(-ViewDir, Normal));

	if (colour.a == 0.0) { discard; }
	else { entityID = v_EntityID; }
}