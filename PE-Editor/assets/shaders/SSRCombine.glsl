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

layout(location = 0) out vec4 f_Result;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(binding = 0) uniform sampler2D Mat_LightingPass;
layout(binding = 1) uniform sampler2D Mat_SSRUVMap;

layout(binding = 2) uniform sampler2D Mat_gViewPosition;
layout(binding = 3) uniform sampler2D Mat_gWorldNormal;
layout(binding = 4) uniform sampler2D Mat_gAlbedo;
layout(binding = 5) uniform sampler2D Mat_gSpecular;
layout(binding = 6) uniform sampler2D Mat_gARM;
layout(binding = 7) uniform sampler2D Mat_gMetadata;
layout(binding = 8) uniform sampler2D Mat_BRDFLut;

vec4 GetReflectionFromUVMap(vec4 sampleUV)
{
	vec4 reflectedColour = texture(Mat_LightingPass, sampleUV.xy);
	float alpha = clamp(sampleUV.b, 0.0, 1.0); // b component of UV map represents SSR accuracy
	return vec4(reflectedColour.rgb * alpha, alpha);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0, float Roughness)
{
	return F0 + (max(vec3(1.0 - Roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
	vec4 sampleUV = texture(Mat_SSRUVMap, v_TexCoords);
	vec3 WorldFragPos = vec3(inverse(u_CameraBuffer.View) * vec4(texture(Mat_gViewPosition, v_TexCoords).rgb, 1.0));
	vec3 WorldNormal = texture(Mat_gWorldNormal, v_TexCoords).rgb;
	vec3 MaterialAlbedo = texture(Mat_gAlbedo, v_TexCoords).rgb;

	vec3 armSample = texture(Mat_gARM, v_TexCoords).rgb;
	float AO = armSample.r;
	float Roughness = 0.0;
	float Metalness = 0.0;
	int LightingModelIndex = int(texture(Mat_gMetadata, v_TexCoords).g); // 0 = blinn-phong, 1 = pbr
	if (LightingModelIndex == 0)
	{
		float SpecularExponent = texture(Mat_gSpecular, v_TexCoords).a;
		Roughness = 1.0 - (SpecularExponent / 256.0);
	}
	else if (LightingModelIndex == 1)
	{
		Roughness = armSample.g;
		Metalness = armSample.b;
	}

	vec3 N = WorldNormal;
	vec3 V = normalize(u_CameraBuffer.ViewPos - WorldFragPos);

	vec4 reflectedColour = GetReflectionFromUVMap(sampleUV);
	float reflectionAlpha = reflectedColour.a;
	vec3 currentColour = texture(Mat_LightingPass, v_TexCoords).rgb;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, MaterialAlbedo, Metalness);

	float NdotV = max(dot(N, V), 0.0);
	vec3 F = FresnelSchlick(NdotV, F0, Roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - Metalness;

	vec3 diffuse = MaterialAlbedo;

	vec2 brdf = texture(Mat_BRDFLut, vec2(NdotV, Roughness)).rg;
	vec3 specular = reflectedColour.rgb * (F * brdf.x + brdf.y);

	vec3 ssrContribution = (kD * diffuse + specular) * AO;

	f_Result = vec4(currentColour + ssrContribution, 1.0);
	vec3 finalLighting = mix(currentColour, ssrContribution, reflectionAlpha);
	f_Result = vec4(finalLighting, 1.0);
}