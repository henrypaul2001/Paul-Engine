// This shader downsamples a texture from a source mip level using the maximum value of surrounding pixels
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
	vec2 texelSize = 1.0 / sourceSize;
	vec2 coords = v_TexCoords;

	vec4 a = texture(Mat_SourceTexture, coords + ivec2(0, 0) * texelSize, sourceMip);
	vec4 b = texture(Mat_SourceTexture, coords + ivec2(1, 0) * texelSize, sourceMip);
	vec4 c = texture(Mat_SourceTexture, coords + ivec2(0, 1) * texelSize, sourceMip);
	vec4 d = texture(Mat_SourceTexture, coords + ivec2(1, 1) * texelSize, sourceMip);

	f_Downsample = max(max(a, b), max(c, d));
}