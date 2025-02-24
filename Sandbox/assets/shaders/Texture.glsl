#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TexCoords;

uniform mat4 u_ViewProjection;

out vec2 v_TexCoords;
out vec4 v_Colour;

void main()
{
	v_TexCoords = a_TexCoords;
	v_Colour = a_Colour;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 v_TexCoords;
in vec4 v_Colour;

uniform sampler2D u_Texture;
uniform vec2 u_TextureScale;
uniform vec4 u_Colour;

void main()
{
	//colour = texture(u_Texture, v_TexCoords * u_TextureScale) * u_Colour;
	colour = v_Colour;
}