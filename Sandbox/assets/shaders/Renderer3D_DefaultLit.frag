#version 450 core
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
layout(location = 1) in VertexData v_VertexData;

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

layout(binding = 0) uniform sampler2DArray DirectionalLightShadowMapArray;
layout(binding = 1) uniform sampler2DArray SpotLightShadowMapArray;
layout(binding = 2) uniform samplerCubeArray PointLightShadowMapArray;

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

vec3 DirectionalLightContribution(int lightIndex, vec3 MaterialAlbedo, vec3 MaterialSpecular, vec3 Normal)
{
	vec3 ambient = u_SceneData.DirLights[lightIndex].Ambient.rgb * MaterialAlbedo;

	// diffuse
	vec3 lightDir = normalize(-u_SceneData.DirLights[lightIndex].Direction.xyz);
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = u_SceneData.DirLights[lightIndex].Diffuse.rgb * diff * MaterialAlbedo;

	// specular
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
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
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
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
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
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

void main()
{
	vec4 Albedo = vec4(1.0, 0.0, 0.0, 1.0);
	vec4 Specular = Albedo;

	vec2 TextureScale = vec2(1.0);
	float Shininess = 16.0;
	float HeightScale = 0.0;

	vec3 EmissionColour = vec3(1.0);
	float EmissionStrength = 0.0;

	int UseNormalMap = 0;
	int UseDisplacementMap = 0;

	ViewDir = normalize(u_CameraBuffer.ViewPos - v_VertexData.WorldFragPos);
	vec3 TangentViewDir = transpose(v_VertexData.TBN) * ViewDir;

	ScaledTexCoords = v_VertexData.TexCoords;
	ScaledTexCoords *= TextureScale;

	// If no texture is provided, these samplers will sample a default 1x1 white texture.
	// This results in no change to the underlying material value when they are multiplied
	vec3 AlbedoSample = vec3(1.0);
	vec3 EmissionSample = vec3(1.0);
	vec3 SpecularSample = vec3(1.0);

	vec3 Normal = normalize(v_VertexData.Normal);

	vec3 MaterialAlbedo = AlbedoSample * Albedo.rgb;
	vec3 MaterialEmission = EmissionSample * (EmissionColour * EmissionStrength);
	vec3 MaterialSpecular = SpecularSample *Specular.rgb;

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

	colour = vec4(Result, Albedo.a);

	// Emission
	colour.rgb += MaterialEmission;

	if (colour.a == 0.0) { discard; }
	else { entityID = v_EntityID; }
}