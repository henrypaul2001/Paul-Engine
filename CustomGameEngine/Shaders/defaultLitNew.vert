#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

out VIEW_DATA {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

out VERTEX_DATA {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;

    mat3 TBN;

    vec3 TangentFragPos;
} vertex_data;

uniform mat4 model;
uniform mat3 normalMatrix;

void main() {
    vertex_data.WorldPos = vec3(model * vec4(aPos, 1.0));
    vertex_data.Normal = normalize(normalMatrix * aNormal);
    vertex_data.TexCoords = aTexCoords;

    // Tangent space
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));

    vertex_data.TangentFragPos = TBN * vertex_data.WorldPos;

    view_data.TangentViewPos = TBN * viewPos;
    view_data.ViewPos = viewPos;

    vertex_data.TBN = TBN;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}