#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VIEW_DATA {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

in VERTEX_DATA {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;

    mat3 TBN;

    vec3 TangentFragPos;
} vertex_data;

struct Material {
    sampler2D TEXTURE_DIFFUSE1;
    sampler2D TEXTURE_SPECULAR1;
    sampler2D TEXTURE_NORMAL1;
    sampler2D TEXTURE_DISPLACE1;

    float SHININESS;
    float HEIGHT_SCALE;

    // non textured material properties
    vec3 DIFFUSE;
    vec3 SPECULAR;

    bool useDiffuseMap;
    bool useSpecularMap;
    bool useNormalMap;
    bool useHeightMap;
};
uniform Material material;
uniform float textureScale;

void main() {
    vec2 TexCoords = vertex_data.TexCoords;
    TexCoords *= textureScale;

    // Position
    gPosition = vertex_data.WorldPos;

    // Normal
    vec3 Normal = vertex_data.Normal;
    if (material.useNormalMap) {
        Normal = texture(material.TEXTURE_NORMAL1, TexCoords).rgb;
        Normal = normalize(vertex_data.TBN * Normal);
    }
    
    gNormal = normalize(Normal);

    // Albedo
    vec3 Albedo = material.DIFFUSE;
    if (material.useDiffuseMap) {
        Albedo = texture(material.TEXTURE_DIFFUSE1, TexCoords).rgb;
    }
    gAlbedoSpec.rgb = Albedo;

    // Specular intensity only, no specular colour yet
    float Specular = material.SHININESS;
    if (material.useSpecularMap) {
        Specular = texture(material.TEXTURE_SPECULAR1, TexCoords).r;
    }
}