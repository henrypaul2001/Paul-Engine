#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// uniform block
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 model;

uniform mat3 normalMatrix;

uniform float textureScale;

void main()
{
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.TexCoords = aTexCoords * textureScale;

    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}