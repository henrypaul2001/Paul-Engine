#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gARM; // Ambient, Roughness, Metalness

in VIEW_DATA {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

in VERTEX_DATA {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;

    mat3 TBN;

    vec3 TangentWorldPos;
} vertex_data;

struct PBRMaterial {
    sampler2D TEXTURE_ALBEDO1;
    sampler2D TEXTURE_NORMAL1;
    sampler2D TEXTURE_METALLIC1;
    sampler2D TEXTURE_ROUGHNESS1;
    sampler2D TEXTURE_AO1;
    sampler2D TEXTURE_DISPLACE1;

    float HEIGHT_SCALE;

    // non textured material properties
    vec3 ALBEDO;
    float METALNESS;
    float ROUGHNESS;
    float AO;

    bool useAlbedoMap;
    bool useNormalMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useAoMap;
    bool useHeightMap;
};
uniform PBRMaterial material;
uniform float textureScale;

vec3 Albedo;
vec3 Normal;
float Metallic;
float Roughness;
float AO;
vec2 TexCoords;

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

vec3 GetNormalFromMap() {
    vec3 tangentNormal = texture(material.TEXTURE_NORMAL1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(vertex_data.WorldPos);
    vec3 Q2 = dFdy(vertex_data.WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    vec2 TexCoords = vertex_data.TexCoords;
    TexCoords *= textureScale;

    vec3 tangentViewDir = normalize(view_data.TangentViewPos - vertex_data.TangentWorldPos);

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
    Normal = vertex_data.Normal;
    if (material.useNormalMap) {
        Normal = GetNormalFromMap();
    }

    // Albedo
    Albedo = material.ALBEDO;
    if (material.useAlbedoMap) {
        Albedo = texture(material.TEXTURE_ALBEDO1, TexCoords).rgb;
    }
    gAlbedo.rgb = Albedo;

    // ARM
    // Get fragment ambient occlusion factor
    AO = material.AO;
    if (material.useAoMap) {
        AO = texture(material.TEXTURE_AO1, TexCoords).r;
    }

    // Get material roughness value
    Roughness = material.ROUGHNESS;
    if (material.useRoughnessMap) {
        Roughness = texture(material.TEXTURE_ROUGHNESS1, TexCoords).r;
    }

    // Get material metalness value
    Metallic = material.METALNESS;
    if (material.useMetallicMap) {
        Metallic = texture(material.TEXTURE_METALLIC1, TexCoords).r;
    }

    gARM.r = AO;
    gARM.g = Roughness;
    gARM.b = Metallic;
}