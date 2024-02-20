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
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat3 normalMatrix;

uniform float textureScale;

void main()
{
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = normalize(normalMatrix * aNormal);
    vs_out.TexCoords = aTexCoords * textureScale;

    //vec3 T = normalize(normalMatrix * aTangent);
    //vec3 N = vs_out.Normal;
    //T = normalize(T - dot(T, N) * N);
    //vec3 B = cross(N, T);
    //vs_out.TBN = transpose(mat3(T, B, N));

    // less efficient - send tbn to fragment which performs matrix multiplications per fragment. Better option is to convert to tangent space here and send those vectors to fragment
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    mat3 TBN = mat3(T, B, N);

    vs_out.TBN = TBN;

    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}