#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;
layout (location = 7) in mat4 aInstancedModelMatrix;
// 8, 9, 10 reserved for instancing

out vec2 TexCoords;

void main() 
{
	TexCoords = aTexCoords;

	gl_Position = vec4(aPos, 1.0);
}