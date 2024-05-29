#version 330 core

struct Material {
    sampler2D TEXTURE_OPACITY1;
    bool useOpacityMap;
};
uniform Material material;

uniform float textureScale;

in vec2 texCoords;

vec2 TexCoords;
float Alpha;

float alphaDiscardThreshold = 0.5;

void main() {
    TexCoords = texCoords;
    TexCoords *= textureScale;

    // Get alpha
    Alpha = 1.0;
    if (material.useOpacityMap) {
        Alpha = texture(material.TEXTURE_OPACITY1, TexCoords).a;
    }


    if (Alpha < alphaDiscardThreshold) {
        discard;
    }
    else {
        // do nothing (write to depth buffer)
    }
}