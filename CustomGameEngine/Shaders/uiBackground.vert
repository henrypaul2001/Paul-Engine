#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;

uniform vec2 translation;

void main() {
	gl_Position = projection * vec4(aPos.xy + translation, -0.1, 1.0);
}