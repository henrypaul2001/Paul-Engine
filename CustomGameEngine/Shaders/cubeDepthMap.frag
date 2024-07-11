#version 330 core
in vec4 FragPos;

struct Material {
    sampler2D TEXTURE_OPACITY1;
    bool useOpacityMap;
    float shadowCastAlphaDiscardThreshold;
};
uniform Material material;

uniform vec2 textureScale;

in vec2 texCoords;

vec2 TexCoords;
float Alpha;

uniform vec3 lightPos;
uniform float far_plane;

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
	    // get distance between fragment and light source
	    float lightDistance = length(FragPos.xyz - lightPos);

	    // map to 0,1 range by dividing by far plane
	    lightDistance = lightDistance / far_plane;

	    // write as modified depth
	    gl_FragDepth = lightDistance;
    }
}