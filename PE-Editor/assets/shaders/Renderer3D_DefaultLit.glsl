#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
} u_CameraBuffer;

layout(std140, binding = 1) uniform MeshSubmission
{
	mat4 Transform;
	int EntityID;
} u_MeshSubmission;

struct ViewData
{
	vec3 ViewPos;
};

struct VertexData
{
	vec3 WorldFragPos;
	vec3 Normal;
	vec2 TexCoords;

	mat3 TBN;
};

layout(location = 0) out flat int v_EntityID;
layout(location = 1) out flat ViewData v_ViewData;
layout(location = 3) out VertexData v_VertexData;

void main()
{
	mat3 normalMatrix = mat3(transpose(inverse(u_MeshSubmission.Transform)));
	v_VertexData.WorldFragPos = vec3(u_MeshSubmission.Transform * vec4(a_Position, 1.0));
	v_VertexData.Normal = normalMatrix * a_Normal;
	v_VertexData.TexCoords = a_TexCoords;

	// Tangent space
	vec3 T = normalMatrix * a_Tangent;
	vec3 N = v_VertexData.Normal;
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	v_VertexData.TBN = mat3(T, B, N);

	v_ViewData.ViewPos = u_CameraBuffer.ViewPos;

	v_EntityID = u_MeshSubmission.EntityID;

	gl_Position = u_CameraBuffer.ViewProjection * vec4(v_VertexData.WorldFragPos, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

const int MAX_ACTIVE_DIR_LIGHTS = 8;
const int MAX_ACTIVE_POINT_LIGHTS = 8;
const int MAX_ACTIVE_SPOT_LIGHTS = 8;

struct ViewData
{
	vec3 ViewPos;
};

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
layout(location = 1) in flat ViewData v_ViewData;
layout(location = 3) in VertexData v_VertexData;

layout(std140, binding = 2) uniform SceneData
{
	DirectionalLight DirLights[MAX_ACTIVE_DIR_LIGHTS];
	PointLight PointLights[MAX_ACTIVE_POINT_LIGHTS];
	SpotLight SpotLights[MAX_ACTIVE_SPOT_LIGHTS];
	int ActiveDirLights;
	int ActivePointLights;
	int ActiveSpotLights;
} u_SceneData;

layout(std140, binding = 3) uniform MaterialValues
{
	vec4 Albedo;
	vec4 Specular;
	vec2 TextureScale;
	float Shininess;
	float HeightScale;
	int UseNormalMap;
	int UseDisplacementMap;
} u_MaterialValues;

layout(binding = 0) uniform sampler2D AlbedoMap;
layout(binding = 1) uniform sampler2D SpecularMap;
layout(binding = 2) uniform sampler2D NormalMap;
layout(binding = 3) uniform sampler2D DisplacementMap;
layout(binding = 4) uniform sampler2DArray DirectionalLightShadowMapArray;
layout(binding = 5) uniform sampler2DArray SpotLightShadowMapArray;

vec2 ScaledTexCoords;
vec3 ViewDir;

float GetShadowFactor(vec4 LightSpaceFragPos, vec3 lightPos, float minBias, float maxBias, vec3 Normal, sampler2DArray shadowMapArray, int shadowMapLayer)
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

float GetShadowFactor(vec4 LightSpaceFragPos, vec3 lightPos, float minBias, float maxBias, vec3 Normal, sampler2D shadowMap)
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
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), tangentViewDir), 0.0));
	float layerDepth = 1.0 / numLayers;

	float currentLayerDepth = 0.0f;

	// amount to shift the texture coordinates per layer
	vec2 P = tangentViewDir.xy * u_MaterialValues.HeightScale;
	vec2 deltaTexCoords = (P / numLayers);

	// initial sample
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(DisplacementMap, currentTexCoords).r;

	while (currentLayerDepth < currentDepthMapValue) {
		// shift coords along direction of P
		currentTexCoords -= deltaTexCoords;

		currentDepthMapValue = texture(DisplacementMap, currentTexCoords).r;

		currentLayerDepth += layerDepth;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for interpolation
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(DisplacementMap, prevTexCoords).r - currentLayerDepth + layerDepth;

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
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), u_MaterialValues.Shininess);
	vec3 specular = (u_SceneData.DirLights[lightIndex].Specular.rgb * spec * MaterialSpecular) * diff;

	// shadow contribution
	float shadowDistance = u_SceneData.DirLights[lightIndex].Ambient.w;
	float minBias = u_SceneData.DirLights[lightIndex].Diffuse.w;
	float maxBias = u_SceneData.DirLights[lightIndex].Specular.w;

	float shadow = 0.0;
	if (bool(u_SceneData.DirLights[lightIndex].Direction.w))
	{
		shadow = GetShadowFactor(u_SceneData.DirLights[lightIndex].LightMatrix * vec4(v_VertexData.WorldFragPos, 1.0), -u_SceneData.DirLights[lightIndex].Direction.xyz * shadowDistance, minBias, maxBias, Normal, DirectionalLightShadowMapArray, lightIndex);
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
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), u_MaterialValues.Shininess);
	vec3 specular = spec * u_SceneData.PointLights[lightIndex].Specular.rgb * MaterialSpecular * diff * attenuation;

	return ambient + diffuse + specular;
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
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), u_MaterialValues.Shininess);
	vec3 specular = spec * u_SceneData.SpotLights[lightIndex].Specular.rgb * MaterialSpecular * diff * attenuation * intensity;

	// shadow contribution
	float minBias = u_SceneData.SpotLights[lightIndex].ShadowData.g;
	float maxBias = u_SceneData.SpotLights[lightIndex].ShadowData.b;

	float shadow = 0.0;
	if (bool(u_SceneData.SpotLights[lightIndex].ShadowData.r))
	{
		shadow = GetShadowFactor(u_SceneData.SpotLights[lightIndex].LightMatrix * vec4(v_VertexData.WorldFragPos, 1.0), lightPos.xyz, minBias, maxBias, Normal, SpotLightShadowMapArray, lightIndex);
	}
	return ambient + (1.0 - shadow) * (diffuse + specular);
}

void main()
{
	ViewDir = normalize(v_ViewData.ViewPos - v_VertexData.WorldFragPos);
	vec3 TangentViewDir = transpose(v_VertexData.TBN) * ViewDir;

	ScaledTexCoords = v_VertexData.TexCoords;
	ScaledTexCoords *= u_MaterialValues.TextureScale;

	// Apply parallax mapping to TexCoords
	if (u_MaterialValues.UseDisplacementMap != 0)
	{
		ScaledTexCoords = ParallaxMapping(ScaledTexCoords, TangentViewDir);
		//ScaledTexCoords = clamp(ScaledTexCoords, 0.0, 1.0);
	}

	// If no texture is provided, these samplers will sample a default 1x1 white texture.
	// This results in no change to the underlying material value when they are multiplied
	vec3 AlbedoSample = texture(AlbedoMap, ScaledTexCoords).rgb;
	vec3 SpecularSample = vec3(texture(SpecularMap, ScaledTexCoords).r);

	vec3 Normal = normalize(v_VertexData.Normal);
	if (u_MaterialValues.UseNormalMap != 0)
	{
		Normal = texture(NormalMap, ScaledTexCoords).rgb;
		Normal = normalize(Normal * 2.0 - 1.0);
		Normal = normalize(v_VertexData.TBN * Normal);
	}

	vec3 MaterialAlbedo = AlbedoSample * u_MaterialValues.Albedo.rgb;
	vec3 MaterialSpecular = SpecularSample * u_MaterialValues.Specular.rgb;

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

	colour = vec4(Result, u_MaterialValues.Albedo.a);

	if (colour.a == 0.0) { discard; }
	else { entityID = v_EntityID; }
}