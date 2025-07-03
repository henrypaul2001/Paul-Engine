// This shader downsamples a texture from a source mip level using the minimum value of surrounding pixels
#context forward
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

layout(location = 0) out vec4 f_Downsample;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform Mat_DownsampleData
{
	int SourceMipLevel;
} u_DownsampleData;

layout(binding = 0) uniform sampler2D Mat_SourceTexture;

void main()
{
	int numMips = textureQueryLevels(Mat_SourceTexture);

	int sourceMip = clamp(u_DownsampleData.SourceMipLevel, 0, numMips - 1);

	ivec2 sourceSize = textureSize(Mat_SourceTexture, sourceMip);
	ivec2 coords = ivec2(vec2(sourceSize) * v_TexCoords);

	vec4 a = texelFetch(Mat_SourceTexture, clamp(coords + ivec2(0, 0), ivec2(0), sourceSize - ivec2(1)), sourceMip);
	vec4 b = texelFetch(Mat_SourceTexture, clamp(coords + ivec2(1, 0), ivec2(0), sourceSize - ivec2(1)), sourceMip);
	vec4 c = texelFetch(Mat_SourceTexture, clamp(coords + ivec2(0, 1), ivec2(0), sourceSize - ivec2(1)), sourceMip);
	vec4 d = texelFetch(Mat_SourceTexture, clamp(coords + ivec2(1, 1), ivec2(0), sourceSize - ivec2(1)), sourceMip);

	f_Downsample = min(min(a, b), min(c, d));
}