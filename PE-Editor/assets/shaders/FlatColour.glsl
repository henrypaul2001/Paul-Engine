// Flat colour shader

#context forward
#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core

layout(location = 0) out vec4 colour;

uniform vec4 u_Colour;

void main()
{
	colour = u_Colour;
}