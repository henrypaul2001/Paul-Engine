#context forward
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in vec2 a_TexScale;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

struct VertexOutput {
	vec4 Colour;
	vec2 TexCoords;
	vec2 TextureScale;
};

layout(location = 0) out VertexOutput Output;
layout(location = 3) out flat float v_TexIndex;
layout(location = 4) out flat int v_EntityID;

void main()
{
	Output.Colour = a_Colour;
	Output.TexCoords = a_TexCoords;
	Output.TextureScale = a_TexScale;

	v_TexIndex = a_TexIndex;
	v_EntityID = a_EntityID;
	gl_Position = u_CameraBuffer.Projection * u_CameraBuffer.View * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

struct VertexOutput {
	vec4 Colour;
	vec2 TexCoords;
	vec2 TextureScale;
};

layout(location = 0) in VertexOutput Input;
layout(location = 3) in flat float v_TexIndex;
layout(location = 4) in flat int v_EntityID;

layout(binding = 0) uniform sampler2D u_Textures[32];

void main()
{
	colour = texture(u_Textures[int(v_TexIndex)], Input.TexCoords * Input.TextureScale) * Input.Colour;

	if (colour.a == 0.0) {
		discard;
	}
	else {
		entityID = v_EntityID;
	}
}