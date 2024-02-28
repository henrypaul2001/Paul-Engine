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
    uniform vec3 viewPos;
};

out VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
    vec3 viewPos;
} vs_out;

uniform mat4 model;

uniform mat3 normalMatrix;

void main()
{
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = normalize(normalMatrix * aNormal);
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N)); //mat3(T, B, N);
    vs_out.TBN = TBN;
    vs_out.viewPos = viewPos;
    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}