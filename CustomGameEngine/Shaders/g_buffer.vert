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
    vertex_data.TexCoords = aTexCoords;
    vertex_data.Normal = normalMatrix * aNormal;
    
    vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
    vertex_data.TBN = transpose(mat3(T, B, N));

    vertex_data.TangentFragPos = vertex_data.TBN * vertex_data.WorldPos;

    view_data.TangentViewPos = vertex_data.TBN * viewPos;
    view_data.ViewPos = viewPos;
    
    gl_Position = projection * view * vec4(vertex_data.WorldPos, 1.0);
}