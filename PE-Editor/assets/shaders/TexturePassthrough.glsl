// Write a texture to framebuffer with no processing
#context forward
#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 1) out vec2 v_TexCoords;

void main()
{
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 f_Colour;

layout(location = 1) in vec2 v_TexCoords;

layout(binding = 0) uniform sampler2D Mat_Input;

void main()
{
	f_Colour = texture(Mat_Input, v_TexCoords);
}