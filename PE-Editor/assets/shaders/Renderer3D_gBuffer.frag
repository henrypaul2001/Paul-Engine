#version 460 core
layout(location = 0) out vec3 gViewPosition;
layout(location = 1) out vec3 gWorldNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gSpecular; // r, g, b = specular colour, a = specular exponent
layout(location = 4) out vec3 gARM; // ao, roughness, metal (PBR)
layout(location = 5) out vec3 gEmission; // pre-multiplied with emission strength
layout(location = 6) out vec2 gMetaData; // (float)entityID, (float)lightingModelIndex

struct VertexData
{
	vec3 WorldFragPos;
	vec3 Normal;
	vec2 TexCoords;

	mat3 TBN;
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

layout(std140, binding = 3) uniform Mat_MaterialValues
{
	vec4 Albedo;
	vec4 Specular;

	vec2 TextureScale;
	float Shininess;
	float HeightScale;

	vec3 EmissionColour;
	float EmissionStrength;

	int UseNormalMap;
	int UseDisplacementMap;
} u_MaterialValues;

layout(binding = 3) uniform sampler2D Mat_AlbedoMap;
layout(binding = 4) uniform sampler2D Mat_SpecularMap;
layout(binding = 5) uniform sampler2D Mat_NormalMap;
layout(binding = 6) uniform sampler2D Mat_DisplacementMap;
layout(binding = 7) uniform sampler2D Mat_EmissionMap;

vec2 ScaledTexCoords;
vec3 ViewDir;

const float minLayers = 0.0;
const float maxLayers = 32.0;
vec2 ParallaxMapping(vec2 texCoords, vec3 tangentViewDir)
{
	// calculate number of depth layers
	float numLayers = mix(maxLayers, minLayers, dot(vec3(0.0, 0.0, 1.0), tangentViewDir));
	float layerDepth = 1.0 / numLayers;

	float currentLayerDepth = 0.0f;

	// amount to shift the texture coordinates per layer
	vec2 P = tangentViewDir.xy * u_MaterialValues.HeightScale;
	vec2 deltaTexCoords = (P / numLayers);

	// initial sample
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(Mat_DisplacementMap, currentTexCoords).r;

	float i = 0.0;
	while (currentLayerDepth < currentDepthMapValue && i < 32.0) {
		// shift coords along direction of P
		currentTexCoords -= deltaTexCoords;

		currentDepthMapValue = texture(Mat_DisplacementMap, currentTexCoords).r;

		currentLayerDepth += layerDepth;
		i += 1.0;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for interpolation
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(Mat_DisplacementMap, prevTexCoords).r - currentLayerDepth + layerDepth;

	// interpolate
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

void main()
{
	ViewDir = normalize(u_CameraBuffer.ViewPos - v_VertexData.WorldFragPos);
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
	vec3 AlbedoSample = pow(texture(Mat_AlbedoMap, ScaledTexCoords).rgb, vec3(u_CameraBuffer.Gamma));
	vec3 EmissionSample = pow(texture(Mat_EmissionMap, ScaledTexCoords).rgb, vec3(u_CameraBuffer.Gamma));
	vec3 SpecularSample = vec3(texture(Mat_SpecularMap, ScaledTexCoords).r);

	vec3 Normal = normalize(v_VertexData.Normal);
	if (u_MaterialValues.UseNormalMap != 0)
	{
		Normal = texture(Mat_NormalMap, ScaledTexCoords).rgb;
		Normal = normalize(Normal * 2.0 - 1.0);
		Normal = normalize(v_VertexData.TBN * Normal);
	}

	vec3 MaterialAlbedo = AlbedoSample * u_MaterialValues.Albedo.rgb;
	vec3 MaterialEmission = EmissionSample * (u_MaterialValues.EmissionColour * u_MaterialValues.EmissionStrength);
	vec3 MaterialSpecular = SpecularSample * u_MaterialValues.Specular.rgb;

	// Write to gBuffer
	gViewPosition = vec3(u_CameraBuffer.View * vec4(v_VertexData.WorldFragPos, 1.0));
	gWorldNormal = Normal;
	gAlbedo = MaterialAlbedo;
	gSpecular = vec4(MaterialSpecular, u_MaterialValues.Shininess);
	gARM = vec3(0.0, 0.0, 0.0);
	gEmission = MaterialEmission;
	gMetaData = vec2(float(v_EntityID), 0.0);
}