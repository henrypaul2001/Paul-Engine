#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewProjection;

out vec2 v_TexCoords;

void main()
{
	v_TexCoords = a_TexCoords;
	gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

void main()
{
	colour = texture(u_Texture, v_TexCoords);
}