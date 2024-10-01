#version 430 core
layout (location = 0) out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D inputImage;

uniform int size = 4;

void main() {
	if (size <= 0) {
		FragColour = texture(inputImage, TexCoords);
		return;
	}

	vec2 texelSize = 1.0 / vec2(textureSize(inputImage, 0));
	vec4 result = vec4(0.0);

	int count = 0;
	for (int x = -size; x < size; x++) {
		for (int y = -size; y < size; y++) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(inputImage, TexCoords + offset);

			count++;
		}
	}

	FragColour = result / count;
}