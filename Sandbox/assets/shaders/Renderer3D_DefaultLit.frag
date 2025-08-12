
#version 450 core

layout(location = 0) out vec4 f_Result;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
} u_CameraBuffer;

layout(binding = 0) uniform sampler2D SourceTexture;

void main()
{
	vec3 sample = texture(SourceTexture, v_TexCoords).rgb;
	vec3 corrected = pow(sample, vec3(1.0 / u_CameraBuffer.Gamma));
	f_Result = vec4(corrected, 1.0);
}