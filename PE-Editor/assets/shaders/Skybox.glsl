#context forward
#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(location = 0) out vec3 v_TexCoords;

void main()
{
	v_TexCoords = a_Position;
	vec4 pos = u_CameraBuffer.Projection * u_CameraBuffer.View * vec4(a_Position, 1.0);
	gl_Position = pos.xyww;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 f_Colour;

layout(location = 0) in vec3 v_TexCoords;

layout(binding = 0) uniform samplerCube Mat_Skybox;

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

void main()
{
	f_Colour = pow(texture(Mat_Skybox, v_TexCoords), vec4(vec3(u_CameraBuffer.Gamma), 1.0));
}