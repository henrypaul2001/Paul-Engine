#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec4 gSpecular;
layout (location = 5) out float gPBRFLAG;
layout (location = 6) out vec4 gViewSpacePos;

in VIEW_DATA {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

in VERTEX_DATA {
    vec3 WorldPos;
    vec3 VertexViewPos;
    vec3 Normal;
    vec2 TexCoords;

    mat3 TBN;

    vec3 TangentWorldPos;
} vertex_data;

struct Material {
    sampler2D TEXTURE_DIFFUSE1;
    sampler2D TEXTURE_SPECULAR1;
    sampler2D TEXTURE_NORMAL1;
    sampler2D TEXTURE_DISPLACE1;
    sampler2D TEXTURE_OPACITY1;

    float SHININESS;
    float HEIGHT_SCALE;

    // non textured material properties
    vec3 DIFFUSE;
    vec3 SPECULAR;

    bool useDiffuseMap;
    bool useSpecularMap;
    bool useNormalMap;
    bool useHeightMap;
    bool useOpacityMap;
};
uniform Material material;
uniform vec2 textureScale;
uniform bool OpaqueRenderPass;

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

vec2 TexCoords;

void WriteToBuffers() {
    // Position
    gPosition = vec4(vertex_data.WorldPos.xyz, 1.0);

    // Vertex View Space
    gViewSpacePos = vec4(vertex_data.VertexViewPos, 1.0);

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

    gPBRFLAG = 0.0;
}

void main() {
    TexCoords = vertex_data.TexCoords;
    TexCoords *= textureScale;

    vec3 tangentViewDir = normalize(view_data.TangentViewPos - vertex_data.TangentWorldPos);

    // Apply parallax mapping to tex coords if material has height map
    if (material.useHeightMap) {
        TexCoords = ParallaxMapping(TexCoords, tangentViewDir);
        if (TexCoords.x > 1.0 || TexCoords.y > 1.0 || TexCoords.x < 0.0 || TexCoords.y < 0.0) {
            //discard;
        }
    }

    // Get alpha
    float Alpha = 1.0;
    if (material.useOpacityMap) {
        Alpha = texture(material.TEXTURE_OPACITY1, TexCoords).a;
    }

    // First pass will render fully opaque pixels. Second pass will render non opaque pixels
    if (OpaqueRenderPass && Alpha >= 1.0) {
        WriteToBuffers();
    }
    else if (!OpaqueRenderPass && Alpha < 1.0) {
        WriteToBuffers();
    }
    else {
        discard;
    }
}