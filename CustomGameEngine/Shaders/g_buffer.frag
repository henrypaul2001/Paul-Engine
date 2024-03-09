#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec4 gSpecular;

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

const float minLayers = 8.0;
const float maxLayers = 32.0;
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    // number of depth layers
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

    // calculate size of each layer
    float layerDepth = 1.0 / numLayers;

    float currentLayerDepth = 0.0;

    // amount to shift the texture coordinates per layer
    vec2 P = viewDir.xy * material.HEIGHT_SCALE;
    vec2 deltaTexCoords = (P / numLayers);

    // get initial values
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(material.TEXTURE_DISPLACE1, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue) {
        // shift coords along direction of P
        currentTexCoords -= deltaTexCoords;

        // get new depth value
        currentDepthMapValue = texture(material.TEXTURE_DISPLACE1, currentTexCoords).r;

        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    
    // get texture coordinates before collision
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(material.TEXTURE_DISPLACE1, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texCoords
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main() {
    vec2 TexCoords = vertex_data.TexCoords;
    TexCoords *= textureScale;

    vec3 tangentViewDir = normalize(view_data.TangentViewPos - vertex_data.TangentFragPos);

    // Apply parallax mapping to tex coords if material has height map
    if (material.useHeightMap) {
        TexCoords = ParallaxMapping(TexCoords, tangentViewDir);
        if (TexCoords.x > 1.0 || TexCoords.y > 1.0 || TexCoords.x < 0.0 || TexCoords.y < 0.0) {
            //discard;
        }
    }

    // Position
    gPosition.xyz = vertex_data.WorldPos;

    // Normal
    vec3 Normal = vertex_data.Normal;
    if (material.useNormalMap) {
        Normal = texture(material.TEXTURE_NORMAL1, TexCoords).rgb;
        Normal = normalize(vertex_data.TBN * Normal);
    }
    gNormal.xyz = normalize(Normal);

    // Albedo
    vec3 Albedo = material.DIFFUSE;
    if (material.useDiffuseMap) {
        Albedo = texture(material.TEXTURE_DIFFUSE1, TexCoords).rgb;
    }
    gAlbedo.rgb = Albedo;

    // Specular
    vec3 Specular = material.SPECULAR;
    if (material.useSpecularMap) {
        Specular = texture(material.TEXTURE_SPECULAR1, TexCoords).rgb;
    }
    gSpecular.rgb = Specular;
    gSpecular.a = material.SHININESS;
}