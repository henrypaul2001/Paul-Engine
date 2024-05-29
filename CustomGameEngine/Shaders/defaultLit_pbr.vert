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
uniform bool instanced;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 8;
uniform mat4 boneTransforms[MAX_BONES];
uniform bool hasBones;

uniform float textureScale;

void main() {
    mat3 NormalMatrix = normalMatrix;
    mat4 Model = model;

    if (instanced) {
        Model = aInstancedModelMatrix;
        NormalMatrix = transpose(inverse(mat3(Model)));
    }

    vec4 transformedLocalPos = vec4(aPos, 1.0);
    vec3 transformedNormal = aNormal;
    vec3 transformTangent = aTangent;
    vec3 transformedBitangent = aBitangent;

    if (hasBones) {
        // Skeletal animation
        mat4 boneTransform = mat4(0.0);
        mat3 boneNormalTransform = mat3(0.0);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (aBoneIDs[i] != -1 && aBoneIDs[i] < MAX_BONES) {
                boneTransform += boneTransforms[aBoneIDs[i]] * aWeights[i];
                boneNormalTransform += mat3(boneTransforms[aBoneIDs[i]]) * aWeights[i];
            }
        }
        transformedLocalPos = boneTransform * vec4(aPos, 1.0);
        transformedNormal = normalize(boneNormalTransform * aNormal);
        transformTangent = normalize(boneNormalTransform * aTangent);
        transformedBitangent = normalize(boneNormalTransform * aBitangent);
    }

    vertex_data.TexCoords = aTexCoords * textureScale;
    vertex_data.WorldPos = vec3(Model * transformedLocalPos);
    vertex_data.Normal = normalize(NormalMatrix * transformedNormal);

    // Tangent space
    vec3 T = normalize(NormalMatrix * transformTangent);
    vec3 N = normalize(NormalMatrix * transformedNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));

    vertex_data.TangentFragPos = TBN * vertex_data.WorldPos;

    view_data.TangentViewPos = TBN * viewPos;

    view_data.ViewPos = viewPos;

    gl_Position = projection * view * vec4(vertex_data.WorldPos, 1.0);
}