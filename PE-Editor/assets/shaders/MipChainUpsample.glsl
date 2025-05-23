// This shader upsamples a HDR texture, taken from the Call of Duty: Advanced Warfare method presented at ACM Siggraph 2014
// Intended to be used to progressively upsample a source texture at each mip level

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

layout(location = 0) out vec3 f_Upsample;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform Mat_UpsampleData
{
	float FilterRadius;
	float AspectRatio;
} u_UpsampleData;

layout(binding = 0) uniform sampler2D Mat_SourceTexture;

void main()
{
	float x = u_UpsampleData.FilterRadius;
	float y = u_UpsampleData.FilterRadius * u_UpsampleData.AspectRatio;

	// Take 9 samples around current texel
	// a - b - c
	// d - e - f
	// g - h - i
	// (e is current texel)

	vec3 a = texture(Mat_SourceTexture, vec2(v_TexCoords.x - x, v_TexCoords.y + y)).rgb;
	vec3 b = texture(Mat_SourceTexture, vec2(v_TexCoords.x, v_TexCoords.y + y)).rgb;
	vec3 c = texture(Mat_SourceTexture, vec2(v_TexCoords.x + x, v_TexCoords.y + y)).rgb;

	vec3 d = texture(Mat_SourceTexture, vec2(v_TexCoords.x - x, v_TexCoords.y)).rgb;
	vec3 e = texture(Mat_SourceTexture, vec2(v_TexCoords.x, v_TexCoords.y)).rgb;
	vec3 f = texture(Mat_SourceTexture, vec2(v_TexCoords.x + x, v_TexCoords.y)).rgb;

	vec3 g = texture(Mat_SourceTexture, vec2(v_TexCoords.x - x, v_TexCoords.y - y)).rgb;
	vec3 h = texture(Mat_SourceTexture, vec2(v_TexCoords.x, v_TexCoords.y - y)).rgb;
	vec3 i = texture(Mat_SourceTexture, vec2(v_TexCoords.x + x, v_TexCoords.y - y)).rgb;

	// Apply weighted distribution, by using a 3x3 tent filter:
	//  1   | 1 2 1 |
	// -- * | 2 4 2 |
	// 16   | 1 2 1 |

	f_Upsample = e * 4.0;
	f_Upsample += (b + d + f + h) * 2.0;
	f_Upsample += (a + c + g + i);
	f_Upsample *= 1.0 / 16.0;
}