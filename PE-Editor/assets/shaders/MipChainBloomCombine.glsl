// This shader is intended to be used to combine the results of the bloom mip chain downsample/upsample passes with an hdr colour buffer
// Also includes option for lens dirt mask

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

layout(location = 0) out vec4 f_Result;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform Mat_BloomCombineData
{
	float BloomStrength;
	float DirtMaskStrength;
	int UseDirtMask; // (bool)
} u_BloomCombineData;

layout(binding = 0) uniform sampler2D Mat_ColourTexture;
layout(binding = 1) uniform sampler2D Mat_BloomTexture;
layout(binding = 2) uniform sampler2D Mat_DirtMaskTexture;

void main()
{
	vec3 colour = texture(Mat_ColourTexture, v_TexCoords).rgb;
	vec3 bloom = texture(Mat_BloomTexture, v_TexCoords).rgb;
	vec3 dirt = vec3(0.0);

	if (bool(u_BloomCombineData.UseDirtMask))
	{
		dirt = texture(Mat_DirtMaskTexture, v_TexCoords).rgb * u_BloomCombineData.DirtMaskStrength;
	}

	f_Result = vec4(mix(colour, bloom + (bloom * dirt), vec3(u_BloomCombineData.BloomStrength)), 1.0);
}