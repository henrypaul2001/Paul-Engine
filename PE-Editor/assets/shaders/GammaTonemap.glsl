// Applies gamma correction to texture

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

layout(binding = 0) uniform sampler2D SourceTexture;

void main()
{
	vec3 hdrColour = texture(SourceTexture, v_TexCoords).rgb;

	// exposure tone mapping
	vec3 mapped = vec3(1.0) - exp(-hdrColour * u_CameraBuffer.Exposure);
	
	// gamma correction
	mapped = pow(mapped, vec3(1.0 / u_CameraBuffer.Gamma));

	f_Result = vec4(mapped, 1.0);
}