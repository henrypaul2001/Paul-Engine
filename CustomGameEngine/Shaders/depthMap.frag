#version 330 core

struct Material {
    sampler2D TEXTURE_OPACITY1;
    bool useOpacityMap;
    float shadowCastAlphaDiscardThreshold;
};
uniform Material material;

uniform float textureScale;

in vec2 texCoords;

vec2 TexCoords;
float Alpha;

void main() {
    TexCoords = texCoords;
    TexCoords *= textureScale;

    // Get alpha
    Alpha = 1.0;
    if (material.useOpacityMap) {
        Alpha = texture(material.TEXTURE_OPACITY1, TexCoords).a;
    }


    if (Alpha < material.shadowCastAlphaDiscardThreshold) {
        discard;
    }
    else {
        // do nothing (write to depth buffer)
    }
}