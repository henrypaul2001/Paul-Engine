// Applies gamma correction to texture

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

vec3 uncharted2_tonemap_partial(vec3 x)
{
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2_filmic(vec3 v)
{
	float exposure_bias = 2.0f;
	vec3 curr = uncharted2_tonemap_partial(v * exposure_bias);

	vec3 W = vec3(11.2f);
	vec3 white_scale = vec3(1.0f) / uncharted2_tonemap_partial(W);
	return curr * white_scale;
}

layout(binding = 0) uniform sampler2D Mat_SourceTexture;

void main()
{
	vec3 hdrColour = texture(Mat_SourceTexture, v_TexCoords).rgb;

	// exposure tone mapping
	vec3 mapped = vec3(1.0) - exp(-hdrColour * u_CameraBuffer.Exposure);
	//vec3 mapped = uncharted2_filmic(hdrColour);

	// gamma correction
	mapped = pow(mapped, vec3(1.0 / u_CameraBuffer.Gamma));

	f_Result = vec4(mapped, 1.0);
}