#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in vec2 a_TexScale;

uniform mat4 u_ViewProjection;

out vec2 v_TexCoords;
out vec4 v_Colour;
out float v_TexIndex;
out vec2 v_TextureScale;

void main()
{
	v_TexCoords = a_TexCoords;
	v_Colour = a_Colour;
	v_TexIndex = a_TexIndex;
	v_TextureScale = a_TexScale;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 v_TexCoords;
in vec4 v_Colour;
in float v_TexIndex;
in vec2 v_TextureScale;

uniform sampler2D u_Textures[32];

void main()
{
	colour = texture(u_Textures[int(v_TexIndex)], v_TexCoords * v_TextureScale) * v_Colour;
}