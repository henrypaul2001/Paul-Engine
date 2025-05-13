// This shader downsamples a HDR texture, taken from the Call of Duty: Advanced Warfare method presented at ACM Siggraph 2014
// Was designed around eliminating pulsating artifacts and temporal stability issues found with other methods
// Intended to be used to progressively downsample a source texture at each mip level

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

layout(location = 0) out vec3 f_Downsample;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform DownsampleData
{
	vec2 SourceResolution;
	float Threshold;
	float SoftThreshold;
	int FirstIteration; // (bool)
	float Gamma;
} u_DownsampleData;

layout(binding = 0) uniform sampler2D SourceTexture;

vec3 PowVec3(vec3 v, float p)
{
	return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 ToSRGB(vec3 v)
{
	return PowVec3(v, 1.0 / u_DownsampleData.Gamma);
}

float RGBToLuminance(vec3 colour)
{
	return dot(colour, vec3(0.2126, 0.71252, 0.0722));
}

float KarisAverage(vec3 colour)
{
	float luma = RGBToLuminance(ToSRGB(colour)) * 0.25;
	return 1.0 / (1.0 + luma);
}

void main()
{
	vec2 srcTexelSize = 1.0 / u_DownsampleData.SourceResolution;
	float x = srcTexelSize.x;
	float y = srcTexelSize.y;

	// Take 13 samples around current texel
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// (e is current texel level)

	vec3 a = texture(SourceTexture, vec2(v_TexCoords.x - 2 * x, v_TexCoords.y + 2 * y)).rgb;
	vec3 b = texture(SourceTexture, vec2(v_TexCoords.x, v_TexCoords.y + 2 * y)).rgb;
	vec3 c = texture(SourceTexture, vec2(v_TexCoords.x + 2 * x, v_TexCoords.y + 2 * y)).rgb;

	vec3 d = texture(SourceTexture, vec2(v_TexCoords.x - 2 * x, v_TexCoords.y)).rgb;
	vec3 e = texture(SourceTexture, vec2(v_TexCoords.x, v_TexCoords.y)).rgb;
	vec3 f = texture(SourceTexture, vec2(v_TexCoords.x + 2 * x, v_TexCoords.y)).rgb;

	vec3 g = texture(SourceTexture, vec2(v_TexCoords.x - 2 * x, v_TexCoords.y - 2 * y)).rgb;
	vec3 h = texture(SourceTexture, vec2(v_TexCoords.x, v_TexCoords.y - 2 * y)).rgb;
	vec3 i = texture(SourceTexture, vec2(v_TexCoords.x + 2 * x, v_TexCoords.y - 2 * y)).rgb;

	vec3 j = texture(SourceTexture, vec2(v_TexCoords.x - x, v_TexCoords.y + y)).rgb;
	vec3 k = texture(SourceTexture, vec2(v_TexCoords.x + x, v_TexCoords.y + y)).rgb;
	vec3 l = texture(SourceTexture, vec2(v_TexCoords.x - x, v_TexCoords.y - y)).rgb;
	vec3 m = texture(SourceTexture, vec2(v_TexCoords.x + x, v_TexCoords.y - y)).rgb;

	// Weighted distribution
	// 0.5 + 0.125 + 0.125 + 0.125 = 1
	// a,b,d,e * 0.125
	// b,c,e,f * 0.125
	// d,e,g,h * 0.125
	// e,f,h,i * 0.125
	// j,k,l,m * 0.5

	// Use karis averaging to help alleviate firefly artifacts with extremely bright subpixels (only needed for first iteration, as following passes will use this pass as input)
	if (bool(u_DownsampleData.FirstIteration))
	{
		vec3 groups[5];
		groups[0] = (a + b + d + e) * (0.125 / 4.0);
		groups[1] = (b + c + e + f) * (0.125 / 4.0);
		groups[2] = (d + e + g + h) * (0.125 / 4.0);
		groups[3] = (e + f + h + i) * (0.125 / 4.0);
		groups[4] = (j + k + l + m) * (0.5 / 4.0);

		groups[0] *= KarisAverage(groups[0]);
		groups[1] *= KarisAverage(groups[1]);
		groups[2] *= KarisAverage(groups[2]);
		groups[3] *= KarisAverage(groups[3]);
		groups[4] *= KarisAverage(groups[4]);
		
		f_Downsample = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];

		float brightness = max(f_Downsample.r, max(f_Downsample.g, f_Downsample.b));
		float knee = u_DownsampleData.Threshold * u_DownsampleData.SoftThreshold;
		float soft = brightness - u_DownsampleData.Threshold + knee;
		soft = clamp(soft, 0, 2.0 * knee);
		soft = soft * soft / (4.0 * knee + 0.00001);

		float contribution = max(soft, brightness - u_DownsampleData.Threshold);
		contribution /= max(brightness, 0.00001);
		f_Downsample *= contribution;
	}
	else
	{
		// 0.125 * 5 + 0.03125 * 4 + 0.0625 * 4 = 1
		f_Downsample = e * 0.125;
		f_Downsample += (a + c + g + i) * 0.03125;
		f_Downsample += (b + d + f + h) * 0.0625;
		f_Downsample += (j + k + l + m) * 0.125;
	}
	f_Downsample = max(f_Downsample, 0.0001);
}