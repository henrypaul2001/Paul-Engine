// designed to be used after a deferred geometry pass, but doesn't contribute to the gBuffer itself
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

layout(location = 0) out vec4 f_Blur;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform Mat_BoxBlurData
{
	int Size;
} u_BoxBlurData;

layout(binding = 0) uniform sampler2D Mat_Input;

void main()
{
	if (u_BoxBlurData.Size <= 0)
	{
		f_Blur = texture(Mat_Input, v_TexCoords).rgba;
		return;
	}
	
	vec2 texelSize = 1.0 / vec2(textureSize(Mat_Input, 0));
	vec4 result = vec4(0.0);

	int size = u_BoxBlurData.Size;
	int count = 0;
	for (int x = -size; x < size; x++)
	{
		for (int y = -size; y < size; y++)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(Mat_Input, v_TexCoords + offset);

			count++;
		}
	}

	f_Blur = result / count;
}