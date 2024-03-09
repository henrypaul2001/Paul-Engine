#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 ViewPos;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

void main() {
	TexCoords = aTexCoords;
    ViewPos = viewPos;
	gl_Position = vec4(aPos, 1.0);
}