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


uniform float textureScale;

void main() {
    vertex_data.TexCoords = aTexCoords * textureScale;
    vertex_data.WorldPos = vec3(model * vec4(aPos, 1.0));
    vertex_data.Normal = normalMatrix * aNormal;

    view_data.ViewPos = viewPos;

    gl_Position = projection * view * vec4(vertex_data.WorldPos, 1.0);
}