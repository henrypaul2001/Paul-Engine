// This shader downsamples a texture from a source mip level using the minimum value of surrounding pixels
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

	f_Downsample = min(min(a, b), min(c, d));
}