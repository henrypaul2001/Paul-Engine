#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

vec4 NoEffect() {
	return texture(screenTexture, TexCoords);
}

vec4 Inversion() {
	return vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
}

vec4 Grayscale() {
	float redWeight = 0.2126;
	float greenWeight = 0.7152;
	float blueWeight = 0.0722;

	vec4 result = texture(screenTexture, TexCoords);
	float average = (redWeight * result.r) + (greenWeight * result.g) + (blueWeight * result.b);

	return vec4(average, average, average, 1.0);
}

const float offset = 1.0 / 300.0;

float[9] HardSharpen = float[](
	-1, -1, -1,
	-1,  9, -1,
	-1, -1, -1
);

float[9] SubtleSharpen = float[](
	 0, -1,  0,
	-1,  5, -1,
	 0, -1,  0
);

float[9] Blur = float[](
	1.0 / 16, 2.0 / 16, 1.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,
	1.0 / 16, 2.0 / 16, 1.0 / 16
);

float[9] EdgeDetect = float[](
	1,  1,  1,
	1, -8,  1,
	1,  1,  1
);

float[9] Emboss = float[](
	-2, -1,  0,
	-1,  1,  1,
	 0,  1,  2
);

float[9] Sobel = float[](
	 1,  2,  1,
	 0,  0,  0,
	-1, -2, -1
);

uniform float postProcessStrength;
uniform unsigned int postProcess;
uniform float[9] customKernel;

vec4 KernelEffect(float[9] kernel) {
	vec2 offsets[9] = vec2[](
		vec2(-offset, offset),  // top-left
		vec2( 0.0f, offset),	// top-center
		vec2( offset, offset),  // top-right
		vec2(-offset, 0.0f),	// center-left
		vec2( 0.0f, 0.0f),		// center-center
		vec2( offset, 0.0f),	// center-right
		vec2(-offset, -offset), // bottom-left
		vec2( 0.0f, -offset),   // bottom-center
		vec2( offset, -offset)  // bottom-right
	);

	vec3 sampleTex[9];
	for (int i = 0; i < 9; i++) {
		sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
	}

	vec3 col = vec3(0.0);
	for (int i = 0; i < 9; i++) {
		col += sampleTex[i] * kernel[i];
	}

	return vec4(col, 1.0);
}

void main() {
	if (postProcess == 0u) {
		FragColor = NoEffect();
	}
	else if (postProcess == 1u) {
		FragColor = Inversion();
	}
	else if (postProcess == 2u) {
		FragColor = Grayscale();
	}
	else if (postProcess == 3u) {
		FragColor = KernelEffect(HardSharpen);
	}
	else if (postProcess == 4u) {
		FragColor = KernelEffect(SubtleSharpen);
	}
	else if (postProcess == 5u) {
		FragColor = KernelEffect(Blur);
	}
	else if (postProcess == 6u) {
		FragColor = KernelEffect(EdgeDetect);
	}
	else if (postProcess == 7u) {
		FragColor = KernelEffect(Emboss);
	}
	else if (postProcess == 8u) {
		FragColor = KernelEffect(Sobel);
	}
	else if (postProcess == 9u) {
		FragColor = KernelEffect(customKernel);
	}

	if (postProcess != 0u) {
		FragColor = vec4(mix(texture(screenTexture, TexCoords).rgb, FragColor.rgb, postProcessStrength), 1.0);
	}

	// gamma correction
	// FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
}