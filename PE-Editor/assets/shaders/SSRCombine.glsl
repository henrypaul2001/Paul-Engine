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
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(binding = 0) uniform sampler2D Mat_LightingPass;
layout(binding = 1) uniform sampler2D Mat_SSRUVMap;

layout(binding = 2) uniform sampler2D Mat_gWorldPosition;
layout(binding = 3) uniform sampler2D Mat_gWorldNormal;

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
	vec3 WorldFragPos = texture(Mat_gWorldPosition, v_TexCoords).rgb;
	vec3 WorldNormal = texture(Mat_gWorldNormal, v_TexCoords).rgb;
	//vec3 Albedo;

	vec3 N = WorldNormal;
	vec3 V = normalize(u_CameraBuffer.ViewPos - WorldFragPos);

	vec4 reflectedColour = GetReflectionFromUVMap(sampleUV);
	float reflectionAlpha = reflectedColour.a;
	vec3 currentColour = texture(Mat_LightingPass, v_TexCoords).rgb;

	// Temporary simplified specular logic
	// TODO: Implement more material detail into these calculations
	float roughness = 0.0; // all surfaces are perfectly smooth for testing
	vec3 F0 = vec3(0.0); // all surfaces are full metal for testing
	vec3 F = FresnelSchlick(max(dot(N, V), 0.0), F0, roughness);
	vec3 kD = 1.0 - F;

	vec3 specular = reflectedColour.rgb * F;
	vec3 ambience = (kD * specular);
	
	currentColour += ambience;

	f_Result = vec4(currentColour, 1.0);
}