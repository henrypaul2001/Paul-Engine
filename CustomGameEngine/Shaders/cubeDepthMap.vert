#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in mat4 aInstancedModelMatrix;

uniform mat4 model;
uniform bool instanced;

void main() {
	mat4 Model = model;
	if (instanced) {
		Model = aInstancedModelMatrix;
	}

	gl_Position = Model * vec4(aPos, 1.0);
}